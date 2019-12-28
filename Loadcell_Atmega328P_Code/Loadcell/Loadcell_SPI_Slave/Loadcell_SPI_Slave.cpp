/*
 * Loadcell_SPI_Slave.cpp
 *
 * Created: 2018-06-14 오전 10:30:12
 *  Author: user
 */ 
#include "../../x328P_Utility/x328Util.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/power.h>

xUnion xdata;

xSPISlave_328 spi;
xHX711 loadCell(24, 23);  //(dataPin, clockPin)
//------------------------------------
ISR(SPI_STC_vect)
{
    spi.interruptHandler();
}

//------------------------------------
uint8_t getCheckSum(uint8_t *buffer, uint8_t len)
{
    uint16_t sum = 0;

    for(uint8_t i=0; i<len; i++) {
        sum += buffer[i];
        if (sum & 0xFF00) sum = (sum & 0x00FF) + 1;
    }

    return (uint8_t)(sum);
}

//----------------------------------------------
int main(void)
{
    if(F_CPU == 8000000UL) clock_prescale_set(clock_div_1);

    sei();


    while(1)
    {
        xdata.u32 = loadCell.read();
        xdata.buffer[4] = ~getCheckSum(xdata.buffer, 4);

        spi.writeBufferMuteEx(xdata.buffer, 4);
  
        spi.blockCount = 0;  //block count increase in slave spi when critical section is blocked by master.
                             //so by checking spi.blockCount you can measure efficiency.

        _delay_ms(10); //This delay is not necessary here because HX711 ADC sampling rate is very low(normally 10SPS).
                      //It can be increased to 80SPS when Rate pin(pin 15) is pulled HIGH.
                      //On HX711 breakboard(Sparkfun), RATE pin connected to GND by default.  
                      //And you can make RATE pin HIGH by cutting the PCB trace line connecting RATE pin to GND.
                      //You can find the cutting point at center of bottom face of PCB.

    }
}