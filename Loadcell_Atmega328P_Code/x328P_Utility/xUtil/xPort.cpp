
#include "../x328Util.h"
//------------------------XX C6 D0 D1 D2 D3 D4 +5 GN B6 B7 D5 D6 D7 B0 B1 B2 B3 B4 B5 AV AR GN C0 C1 C2 C3 C4 C5
//------------------------XX  1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28
int8_t xPort::pinMap[] = {-1, 6, 0, 1 ,2, 3, 4,-1,-1, 6, 7, 5, 6, 7, 0, 1, 2, 3, 4, 5,-1,-1,-1, 0, 1, 2, 3, 4, 5};

xPort::xPort()
{
   ddr = 0;
   outPort = 0;
   inPort = 0;
   bitMask = 0;
}

xPort::xPort(uint8_t pinNo)
{
   
   if((pinNo >= 2 && pinNo <= 6) || (pinNo >= 11 && pinNo <= 13)) {
      ddr = &DDRD;
      outPort = &PORTD;
      inPort = &PIND;
      bitMask = 1 << pinMap[pinNo];
   }
   else if((pinNo >= 9 && pinNo <= 10) || (pinNo >= 14 && pinNo <= 19)) {
      ddr = &DDRB;
      outPort = &PORTB;
      inPort = &PINB;
      bitMask = 1 << pinMap[pinNo];
   }
   else if((pinNo ==1) || (pinNo >= 23 && pinNo <= 28)) {
      ddr = &DDRC;
      outPort = &PORTC;
      inPort = &PINC;
      bitMask = 1 << pinMap[pinNo];
   }
   else {
      ddr = 0;
      outPort = 0;
      inPort = 0;
      bitMask = 0;
   }
}

//-------------------------------
//void xPort::DDR(uint8_t x)
//{
   //if(!bitMask) return;
   //
   //if(x) *ddr |=  bitMask;
   //else  *ddr &= ~bitMask;
//}

//-------------------------------
void xPort::put(uint8_t x)
{
   if(!bitMask) return;
   if(!((*ddr) & bitMask)) *ddr |= bitMask;
   
   if(x) *outPort |=  bitMask;
   else  *outPort &= ~bitMask;
}

//-------------------------------
uint8_t xPort::get()
{
   if(!bitMask) return 0;
   if((*ddr) & bitMask) *ddr &= ~bitMask;
   return ((*inPort) & bitMask) ? 1 : 0;
}

//-------------------------------
void xPort::toggle()
{
   *outPort ^= bitMask;
}

//-------------------------------
void xPort::wait_until(uint8_t x)
{
   if(!bitMask) return;
   
   uint8_t b;
   
   while(1) {
      b = (*inPort) & bitMask;
      if(x && b) return;
      if((!x) && (!b)) return;
   }

}

//------------------------------
void xPort::setPCINT()
{
   if(ddr == &DDRB)  {
      PCICR |= (1 << PCIE0);  //set B bank
      PCMSK0 |= bitMask;
   }
   else if(ddr == &DDRC) {
      PCICR |= (1 << PCIE1);  //set C bank
      PCMSK1 |= bitMask;
   }
   else if(ddr == &DDRD) {
      PCICR |= (1 << PCIE2);  //set D bank
      PCMSK2 |= bitMask;
   }
}

//------------------------------
void xPort::clearPCINT()
{
   if(ddr == &DDRB)  {
      PCMSK0 &= ~bitMask;
      if(PCMSK0==0) PCICR &= ~(1 << PCIE0);  //clear B bank
   }
   else if(ddr == &DDRC) {
      PCMSK1 &= ~bitMask;
      if(PCMSK1==0) PCICR &= ~(1 << PCIE1);  //clear C bank
   }
   else if(ddr == &DDRD) {
      PCMSK2 &= ~bitMask;
      if(PCMSK2==0) PCICR &= ~(1 << PCIE2);  //clear D bank
   }
}