
#ifndef XADC_H_2017_02_04_14_31_22
#define XADC_H_2017_02_04_14_31_22
//----------------------------------------------
#include <avr/io.h>

#define ADC_EXTERNAL_AREF (0XFF)
#define ADC_EXTERNAL_AVCC (1<<REFS0)
#define ADC_INTERNAL_1V1  ((1<<REFS1) | (1<<REFS0))

#define ADC_PRESCALE_DIV_2   (1<<ADPS0)                 //001
#define ADC_PRESCALE_DIV_4   (1<<ADPS1)                 //010
#define ADC_PRESCALE_DIV_8   ((1<<ADPS1) | (1<<ADPS0))  //011
#define ADC_PRESCALE_DIV_16  (1<<ADPS2)                 //100
#define ADC_PRESCALE_DIV_32  ((1<<ADPS2) | (1<<ADPS0))  //101
#define ADC_PRESCALE_DIV_64  ((1<<ADPS2) | (1<<ADPS1))  //110
#define ADC_PRESCALE_DIV_128 ((1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0)) //111

class xADC
{
   static bool enabled;
public:
   xADC();
   xADC(uint8_t refVolt);
   xADC(uint8_t refVolt,  uint8_t preScale);
   static void init(uint8_t refVolt,  uint8_t preScale);
   static void enable();
   static void disable();
   static uint16_t readSingle(uint8_t channel, uint8_t overSampling=1);
};

//----------------------------------------------
#endif /* XADC_H_2017_02_04_14_31_22 */