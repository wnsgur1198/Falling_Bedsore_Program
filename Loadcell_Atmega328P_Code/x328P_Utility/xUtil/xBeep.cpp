
#include "../x328Util.h"
#include <avr/io.h>
#include <util/delay.h>

void xBeep::beepUp(xPort x)
{
   uint16_t key[] = {375, 298, 251};
   beep(x, key);
}

void xBeep::beepDown(xPort x)
{
   uint16_t key[] = {251, 298, 375};
   beep(x, key);
}

void xBeep::beep(xPort x, uint16_t key[])
{
   uint32_t duration = 200000;

   //for(uint8_t k=0; k<(sizeof(key)/sizeof(uint16_t)); k++) {
   for(uint8_t k=0; k<3; k++) {
      uint16_t wavelength = key[k];
      
      for (uint32_t elapsed = 0; elapsed < duration; elapsed += wavelength) {
         /* For loop with variable delay selects the pitch */
         for (uint16_t i = 0; i < wavelength; i++) {
            _delay_us(1);
         }
         x.toggle();
      }
   }
}