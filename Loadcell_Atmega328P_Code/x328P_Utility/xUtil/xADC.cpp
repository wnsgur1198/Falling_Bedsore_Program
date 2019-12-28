
#include "../x328Util.h"
#include <avr/io.h>

bool xADC::enabled = false;

//-------------------------------------
xADC::xADC()
{
    uint8_t preScale = ADC_PRESCALE_DIV_8;

    if(F_CPU == 8000000UL) preScale = ADC_PRESCALE_DIV_64;
    else if(F_CPU == 16000000UL) preScale = ADC_PRESCALE_DIV_128;

    init(ADC_EXTERNAL_AVCC, preScale);
}

//-------------------------------------
xADC::xADC(uint8_t refVolt)
{
    uint8_t preScale = ADC_PRESCALE_DIV_8;  //F_CPU == 1000000UL;

    if(F_CPU == 8000000UL) preScale = ADC_PRESCALE_DIV_64;
    else if(F_CPU == 16000000UL) preScale = ADC_PRESCALE_DIV_128;


    init(refVolt, preScale);
}

//-------------------------------------
xADC::xADC(uint8_t refVolt,  uint8_t preScale)
{
   init(refVolt, preScale);
}

//-------------------------------------
void xADC::init(uint8_t refVolt,  uint8_t preScale)
{
   uint8_t refMsk = (1<<REFS1) | (1<<REFS0);
   uint8_t psMsk  = (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
   
   ADMUX  = (ADMUX  & ~refMsk) | (refVolt & refMsk);
   ADCSRA = (ADCSRA & ~psMsk)  | (preScale & psMsk);  //분주비 50KHZ ~ 200KHz
   enable();
}

//-------------------------------------
void xADC::enable()
{
   ADCSRA |= (1<<ADEN);
   enabled = true;
}

//-------------------------------------
void xADC::disable()
{
   ADCSRA &= ~(1<<ADEN);
   enabled = false;
}

//-------------------------------------
uint16_t xADC::readSingle(uint8_t channel, uint8_t overSampling)
{
   if(!enabled) return 0;
   
   ADMUX = (ADMUX & (~0x0F)) | (channel & 0x0F);  //ADC채널 선택
   
   int16_t sum = 0;
   
   for(uint8_t i=0; i<=overSampling; i++) {
      ADCSRA |= (1<<ADSC); //측정 시작
      loop_until_bit_is_clear(ADCSRA, ADSC); //측정이 끝날 때까지 대기
      if(i) sum += ADC;
   }
   return (sum/overSampling);
}
//-------------------------------------