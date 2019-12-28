#include "../x328Util.h"
#include <avr/io.h>
#include <util/delay.h>


xSPIMaster::xSPIMaster() {}

xSPIMaster::xSPIMaster(uint8_t preScaler, uint8_t dataMode)
{
      // Set MOSI, SCK and SS output, all others input
      DDRB |= (1<<3)|(1<<5); // | (1<<2); // if SS pin is set as Input then SS pin must be pulled HIGH

      // Enable SPI, Master, set clock rate (system_clock/preScaler)
      
      SPCR = (1<<SPE)|(1<<MSTR)|(dataMode << 2);  //dataMode=0neOf{0,1,2,3} (CPOL,CPHA)
      
      if     ((preScaler==8)  || (preScaler==16)) SPCR |= (1<<SPR0);
      else if((preScaler==32) || (preScaler==64)) SPCR |= (1<<SPR1);
      else if((preScaler==128)) SPCR |= (1<<SPR0) | (1<<SPR1);
      
      if((preScaler==2) || (preScaler==8) || (preScaler==32)) SPSR |= (1<<SPI2X);  //double the speed

}

uint8_t xSPIMaster::swap(uint8_t x)
{
    // Start transmission
    SPDR = x;

    // Wait for transmission complete
    while(!(SPSR & (1<<SPIF))) ;

    return SPDR;
}

uint8_t xSPIMaster::selectWrite(uint8_t x)
{
    uint8_t receiveValue;
    
    start();  //select slave
    
    receiveValue=swap(x);
    
    stop();  //release slave
    
    return receiveValue;
}


