

#ifndef XBEEP_H_2017_02_04_16_53_37
#define XBEEP_H_2017_02_04_16_53_37
//-------------------------------------------------

#include <avr/io.h>
#include "xPort.h"

class xBeep
{
public:
   static void beepUp(xPort x);
   static void beepDown(xPort x);
   static void beep(xPort x, uint16_t *key);
};
//-------------------------------------------------
#endif /* XBEEP_H_2017_02_04_16_53_37 */