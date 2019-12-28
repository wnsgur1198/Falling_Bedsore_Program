#include "../x328Util.h"
#include <avr/io.h>
#include <avr/interrupt.h>

xSPISlave::xSPISlave() 
{
    initSlave(true);
}

xSPISlave::xSPISlave(bool interruptEnable)
{
    initSlave(interruptEnable);
}

void xSPISlave::initSlave(bool interruptEnable)
{
    /* Set MISO output, all others input */
    DDRB |= (1<<4);
    
    SPDR=0;
    
    /* Enable SPI */
    SPCR = (1<<SPE);
    
    interruptDriven = interruptEnable;
    
    if(interruptEnable) {

        SPCR |= (1<<SPIE);
        sei();
    }

}
