#include "../x328Util.h"
#include <avr/io.h>
#include <util/delay.h>
#include <math.h>


xLCD::xLCD(uint8_t rs_pin, uint8_t en_pin, uint8_t d4_pin, uint8_t d5_pin, uint8_t d6_pin, uint8_t d7_pin)
{
   rs = xPort(rs_pin);
   en = xPort(en_pin);
   d4 = xPort(d4_pin);
   d5 = xPort(d5_pin);
   d6 = xPort(d6_pin);
   d7 = xPort(d7_pin);
   
   //rs.DDR(1);
   //en.DDR(1);
   //d4.DDR(1);
   //d5.DDR(1);
   //d6.DDR(1);
   //d7.DDR(1);
   

   _delay_ms(100);    // Power-up delay : initial 40 mSec delay

   rs.put(0);  //명령어 모드(레지스터) 선택

   // 초기화
   put4bits(LCD_8_1_5X7_MODE);   // first part of reset sequence
   _delay_ms(10);                // 4.1 mS delay (min)

   put4bits(LCD_8_1_5X7_MODE);   // second part of reset sequence
   _delay_us(200);               // 100uS delay (min)

   put4bits(LCD_8_1_5X7_MODE);   // third part of reset sequence
   _delay_us(200);               // this delay is omitted in the data sheet


   //4비트 모드로 전환(현재는 8비트모드)
   put4bits(LCD_4_2_5X7_MODE);   // set 4-bit mode //우선 상위 4비트에 의해서 4비트모드로 바꾸고
   _delay_us(80);                // 40uS delay (min)

   control(LCD_4_2_5X7_MODE);    // 4비트모드에서 나머지 하위 4비트에 의한 row수, 폰트 크기 지정
   _delay_us(80);                // 40uS delay (min)

   //--아래는 메뉴얼에 따른 나머지 초기화 명령어들(필요 없으면 삭제)
   control(LCD_DISPLAY_OFF_CURSOR_OFF);  // 디스플레이를 끄고
   _delay_us(80);                        // 40uS delay (min)

   control(LCD_CLEAR);                   // 화면을 지우고
   _delay_ms(4);                         // 1.64 mS delay (min)

   control(LCD_OUTPUT_RIGHT);            // 문자출력은 왼쪽에서 오른쪽으로 지정
   _delay_us(80);                        // 40uS delay (min)
   
   control(LCD_DISPLAY_ON_CURSOR_OFF);   // 다시 디스플레이를 켠다.
   _delay_us(80);
   
   
}

//--------------------------------------
void xLCD::put4bits(uint8_t x)
{
   en.put(0); //disable
   _delay_us(1);
   
   d7.put(x & (1<<7));
   d6.put(x & (1<<6));
   d5.put(x & (1<<5));
   d4.put(x & (1<<4));
   
   en.put(1);  //enable
   _delay_us(1);
   
   en.put(0);  //disable
   _delay_us(1);  
  
}

//--------------------------------------
void xLCD::control(uint8_t x)
{
   rs.put(0);  //명령어 레지스터 선택

   put4bits(x);        // write the upper 4-bits of the data
   put4bits(x << 4);   // write the lower 4-bits of the data
}

//--------------------------------------
void xLCD::put(char x)
{
   rs.put(1);  //데이터 레지스터 선택

   put4bits(x);                           // write the upper 4-bits of the data
   put4bits(x << 4);                      // write the lower 4-bits of the data
}
//------------------------------------
void xLCD::put(int8_t x)
{
   put((int32_t)x);
}
//------------------------------------
void xLCD::put(int16_t x)
{
   put((int32_t)x);
}
//------------------------------------
void xLCD::put(int32_t x)
{
   if(x < 0) {
      put((char)'-');
      x = -x;
   }
   
   put((uint32_t)x);
}
//------------------------------------
void xLCD::put(uint8_t x)
{
   put((uint32_t)x);
}
//------------------------------------
void xLCD::put(uint16_t x)
{
   put((uint32_t)x);
}
//--------------------------------------
void xLCD::put(uint32_t x)
{

   uint8_t digits[12];
   int16_t i=0;
   
   if(x == 0) {
      put((char)'0');
      return;
   }
   
   for(i=0; x; i++) {
      digits[i] = x % 10;
      x /= 10;
   }
   
   //putln();
   
   for(i--; i>=0; i--) {
      put((char)(digits[i]+'0'));
   }
}
//------------------------------------
void xLCD::put(float x)
{
   put((double)x);
}
//------------------------------------
void xLCD::put(double x)
{
   if(x < 0.0) {
      put((char)'-');
      x = -x;
   }

   x = (double)(((uint32_t)round(x * 100.0)) / 100.0);
   
   put((uint32_t)x);
   put((char)'.');
   put(((uint32_t)(x * 100.0)) % 100);
}

//--------------------------------------
void xLCD::put(const char *str)
{                       
   for(uint8_t i=0; str[i]; i++)
   {
      put(str[i]);
      _delay_us(80);                              // 40 uS delay (min)
   }
}

//--------------------------------------
void xLCD::gotoXY(uint8_t x, uint8_t y)
{
   uint8_t line = (y) ? LCD_LINE_2_HOME : LCD_LINE_1_HOME;
   control(line | x);
   _delay_ms(4);
}

//--------------------------------------
void xLCD::clear()
{
   control(LCD_CLEAR);
   _delay_ms(4);
}

//--------------------------------------