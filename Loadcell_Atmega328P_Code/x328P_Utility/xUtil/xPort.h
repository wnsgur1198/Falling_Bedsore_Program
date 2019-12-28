#ifndef XPORT_H_2017_01_27_18_47_00
#define XPORT_H_2017_01_27_18_47_00
//-------------------------------------------------

#include <avr/io.h>

class xPort
{
   static int8_t pinMap[29];
   volatile uint8_t *ddr;     //DDRx
   volatile uint8_t *outPort; //output register
   volatile uint8_t *inPort;  //input register
   uint8_t  bitMask;  //bit mask  
    
public:
   xPort();
   xPort(uint8_t pinNo);
   //void DDR(uint8_t x);
   void put(uint8_t x);
   uint8_t get();
   void toggle();
   void wait_until(uint8_t x);
   void setPCINT();
   void clearPCINT();
};

typedef xPort xPin;

#endif /* XPORT_H_2017_01_27_18_47_00 */