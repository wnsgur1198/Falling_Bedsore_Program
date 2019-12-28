#include "xUART.h"
#include <avr/io.h>
#include <util/setbaud.h>
#include <math.h>

bool xUART::isInitialized = false;

//------------------------------------
xUART::xUART()
{
    init(false);
}

//------------------------------------
xUART::xUART(bool receiveInterrupt)
{
   init(receiveInterrupt);
}

//-----------------------------------
void xUART::init(bool receiveInterrupt)
{
   if(isInitialized) return;
   
   UBRR0H = UBRRH_VALUE;
   UBRR0L = UBRRL_VALUE;
   
   #if USE_2X
   UCSR0A |= (1 << U2X0);
   #else
   UCSR0A &= ~(1 << U2X0);
   #endif
   
   UCSR0B = (1 << TXEN0) | (1 << RXEN0) | ((receiveInterrupt) ? (1 << RXCIE0) : 0);   
   UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8 data bits, 1 stop bit
   
   isInitialized = true;
}

//------------------------------------
uint8_t xUART::isBufferReady()
{
   if(!isInitialized) return 0;
   
   return (UCSR0A & (1 << UDRE0));
}

//-------------------------------------
uint8_t xUART::isDataReady()
{
   if(!isInitialized) return 0;
   return (UCSR0A & (1 << RXC0));
}

//------------------------------------
void xUART::transmitByte(uint8_t x)
{
   if(!isInitialized) return;
   loop_until_bit_is_set(UCSR0A, UDRE0);
   UDR0 = x;
}
//------------------------------------
uint8_t xUART::receiveByte()
{
   if(!isInitialized) return 0;
   loop_until_bit_is_set(UCSR0A, RXC0);
   return UDR0;
}

//------------------------------------
void xUART::transmitBuffer(uint8_t *buff, int16_t len)
{
   if(!isInitialized) return;
   for(int16_t i=0;i<len; i++) {
      transmitByte(buff[i]);
   }
}

//------------------------------------
void xUART::receiveBuffer(uint8_t *buff, int16_t len)
{
   if(!isInitialized) return;
   for(int16_t i=0;i<len; i++) {
      buff[i] = receiveByte();
   }
}

//------------------------------------
void xUART::transmitData(xUnion xdu, uint8_t len)
{
   if(!isInitialized) return;  
   
   for(uint8_t i=0; i<len; i++) {
      transmitByte(xdu.buffer[i]);
   }
}

//------------------------------------
xUnion xUART::receiveData(uint8_t len)
{
   
   xUnion xdu;
   
   if(!isInitialized) {
      xdu.u8 = 0;
      return xdu;
   }      
   
   for(uint8_t i=0; i<len; i++) {
      xdu.buffer[i] = receiveByte();
   }
   
   return xdu;
}

//------------------------------------
void xUART::put(char x)
{
   if(!isInitialized) return;
   transmitByte((uint8_t)x);
}

//------------------------------------
void xUART::put(const char *str)
{
   if(!isInitialized) return;
   for(int16_t i=0; str[i]; i++) {
      transmitByte(str[i]);
   }
}

//------------------------------------
void xUART::put(int8_t x)
{
   put((int32_t)x);
}
//------------------------------------
void xUART::put(int16_t x)
{
   put((int32_t)x);
}
//------------------------------------
void xUART::put(int32_t x)
{
   if(!isInitialized) return;

   if(x < 0) {
      transmitByte('-');
      x = -x;
   }
   
   put((uint32_t)x);
}

//------------------------------------
void xUART::put(uint8_t x)
{
   put((uint32_t)x);
}
//------------------------------------
void xUART::put(uint16_t x)
{
   put((uint32_t)x);
}
//------------------------------------
void xUART::put(uint32_t x)
{
   if(!isInitialized) return;
   uint8_t digits[12];
   int16_t i=0;
   
   if(x == 0) {
      transmitByte('0');
      return;
   }
   
   for(i=0; x; i++) {
      digits[i] = x % 10;
      x /= 10;
   }
   
   //putln();
   
   for(i--; i>=0; i--) {
      transmitByte(digits[i]+'0');
   }
}

//------------------------------------
void xUART::put(float x)
{
   put((double)x);
}
//------------------------------------
void xUART::put(double x)
{
   if(!isInitialized) return;
   if(x < 0.0) {
      transmitByte('-');
      x = -x;
   }

   x = (double)(((uint32_t)round(x * 100.0)) / 100.0);
   
   put((uint32_t)x);
   transmitByte('.');
   put(((uint32_t)(x * 100.0)) % 100);
}

//------------------------------------
void xUART::putln()
{
   if(!isInitialized) return;
   transmitByte('\r');
   transmitByte('\n');
}

//------------------------------------
void xUART::putBin(uint8_t x)
{
   if(!isInitialized) return;
   for (int8_t bit = 7; bit >= 0; bit--) {
      
      if (x & (1<<bit)) transmitByte('1');
      else              transmitByte('0');
      
   }//for
}

//------------------------------------
void xUART::putHex(uint8_t x)
{
   if(!isInitialized) return;
   uint8_t nibble;

   nibble = x >> 4;
   if (nibble < 10) transmitByte('0' + nibble);
   else             transmitByte('A' + (nibble - 10));
   
   nibble = (x & 0x0F);
   if (nibble < 10) transmitByte('0' + nibble);
   else             transmitByte('A' + (nibble - 10));
}

//------------------------------------
char xUART::getChar()
{
   if(!isInitialized) return 0;
   
   uint8_t echox = 0;
   uint8_t x;
   
   for(;;) {
      x = receiveByte();
      if(x==0x7F) {
         if(echox != 0) transmitByte(x);
         echox = 0;
         continue;
      }
      else if(x=='\n') continue;
      else if(x=='\r') break;
      else if(x < 0x20 || x >0x7E) continue;
      
      if(echox != 0) transmitByte(0x7F);
      transmitByte(echox=x);
   }
   putln();
   return (char)echox;
}

//------------------------------------
void xUART::getStr(char *str, uint16_t maxLength)
{
   if(!isInitialized) return;
   uint8_t x;
   uint16_t i;
   
   for(i=0; ;) {
      
      x=receiveByte();
      
      if (x == 0x7F) {
         if(i>0) {
            i--;
            transmitByte(x);
         }
      }
      else if( x == '\n') continue;
      else if( x == '\r') break;
      else if( i<maxLength) {
         if((x >= 0x20) && (x <= 0x7E)) {
            str[i++] = x;
            transmitByte(x);
         }
      }
   }//for
   
   putln();
   str[i] = 0;
}

//-----------------------------------
int16_t xUART::getInt16()
{
   if(!isInitialized) return 0;
   xDoubleStruct ds = get_Double_Struct(5,0);
   return (int16_t)(ds.sign * ds.intPart);
}

//-----------------------------------
int32_t xUART::getInt32()
{
   if(!isInitialized) return 0;
   xDoubleStruct ds = get_Double_Struct(10,0);
   return (ds.sign * ds.intPart);
}

//------------------------------------
float xUART::getFloat()
{
   if(!isInitialized) return 0.0;
   xDoubleStruct ds = get_Double_Struct(5,3);
   return ((float)ds.sign) * ((float)ds.intPart + ((float)ds.fracPart / (float)ds.fracExp));
}

//================================================
//private members
xDoubleStruct xUART::get_Double_Struct(uint8_t intLen, uint8_t fracLen)
{

   uint8_t digits[intLen+fracLen+3]; // 3: 1 for dot, 1 for NULL, 1 for sign
   
   int16_t i, dot, count;
   uint8_t x;
   
   dot = -1;
   
   for(i=0; ; ) {
      
      x = receiveByte();
      
      if(x == 0x7F) {  // Backspace
         if(i > 0) {
            i--;
            if(digits[i] == '.') dot = -1; // no dot
            transmitByte(x);
         }
      }
      
      else if(x == '-') {
         if(i==0) {
            digits[i++] = x;
            transmitByte(x);
         }
      }
      
      else if(x == '.') {
         if(fracLen == 0) continue;
         if(dot < 0) {
            dot = i;  //allows only one dot
            digits[i++] = x;
            transmitByte(x);
         }
      }
      else if(x == '\n') continue;
      else if(x == '\r') {
         putln();
         break;
      }
      
      else if(x < '0' || x > '9') continue;
      else {
         
         if(dot < 0) {
            count = (digits[0]=='-') ? (i-1) : i;
            if(count >= intLen) continue;
         }
         else {
            count = i-dot-1;
            if(count >= fracLen) continue;
         }
         
         digits[i++] = x;
         transmitByte(x);
      }
      
   }  // until nextLine and CarriageReturn
   
   digits[i] = 0;
   
   int8_t sgn = 1;
   int32_t sum = 0; 
   int32_t expn = 1;

   xDoubleStruct doubleStruct;
  
   
   for(dot=0; digits[dot]; dot++) {
      if(digits[dot] == '.') break;
   }
   
   // get fractional part ---
   expn = 1;  sum = 0;
   for(i--; i>dot; i--) {
      sum += ((digits[i] - '0') * expn);
      expn *= 10;
   }
   doubleStruct.fracPart = sum;
   doubleStruct.fracExp = expn;
   
   // get integer part ---
   expn = 1; sum = 0;
   for(i=dot-1; i>=0; i--) {
      if(digits[i] == '-') {
         sgn = -1;
      }
      else {
         sum += ((digits[i] - '0') * expn);
         expn *= 10;
      }
   }
   doubleStruct.intPart = sum;
   //doubleStruct.intExp = expn;
   doubleStruct.sign = sgn;
   
   return doubleStruct;
   
}
//------------------------------------