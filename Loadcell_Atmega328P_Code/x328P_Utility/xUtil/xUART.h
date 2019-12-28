
#ifndef XUART_H_2017_01_20_20_24_00
#define XUART_H_2017_01_20_20_24_00
//-------------------------------------------------

#include <avr/io.h>

//value = sign * (intPart + ((double)fracPart / (double)fracExp));
class xDoubleStruct
{
public:
   int32_t intPart;
   int32_t fracPart; 
   int32_t fracExp;
   int8_t  sign;  //1: plus, -1: minus
};

class xUART
{
   static bool isInitialized;
   static xDoubleStruct get_Double_Struct(uint8_t intLen, uint8_t fracLen);
   
public:
    
   xUART();
   xUART(bool receiveInterrupt);
   static void init(bool receiveInterrupt);
   
   static uint8_t isBufferReady();
   static uint8_t isDataReady(); 
   
   static void transmitByte(uint8_t x);
   static uint8_t receiveByte();
   
   static void transmitBuffer(uint8_t *buff, int16_t len);
   static void receiveBuffer(uint8_t *buff, int16_t len);
   
   static void transmitData(xUnion xdu, uint8_t len);
   static xUnion receiveData(uint8_t len);
   
   static void put(char x);
   static void put(const char *str);
   
   static void put(int8_t x);
   static void put(uint8_t x);
   
   static void put(int16_t x);
   static void put(uint16_t x);
   
   static void put(int32_t x);
   static void put(uint32_t x);
   
   static void put(float x);
   static void put(double x);
   
   static void putln();
   static void putBin(uint8_t x);
   static void putHex(uint8_t x);
   
   static char getChar();
   static void getStr(char *str, uint16_t maxLength);
   static int16_t getInt16();
   static int32_t getInt32();
   static float getFloat();

};
//-------------------------------------------------
#endif /* XUART_H_2017_01_20_20_24_00 */