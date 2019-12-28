
#ifndef XSPIMASTER_328_H_2018_06_06_10_45_00
#define XSPIMASTER_328_H_2018_06_06_10_45_00
//--------------------------------------------

#include <avr/io.h>
#include "xSPIMaster.h"

//#define HIGH (1)
//#define LOW  (0)

class xSPIMaster_328
{
    xSPIMaster spi;

    void enterCriticalSection();
    
    void leaveCriticalSection();

    void sendCommand(uint8_t x)
    {
        spi.selectWrite(x);
    }

public:
    
    uint32_t blockCount;
    
    xSPIMaster_328(uint8_t preScaler=8, uint8_t dataMode=0);
    
    void selectSlave(xPort p)
    {
        spi.selectSlave(p);
    }

    //uint8_t read()
    //{
        //return spi.selectRead();
    //}
//
    //void write(uint8_t x)
    //{
        //spi.selectWrite(x);
    //}
    
    void readBuffer(uint8_t *buffer, uint8_t len);

    void writeBuffer(uint8_t *buffer, uint8_t len);
 
    bool readBufferMuteEx(uint8_t *buffer, uint8_t len);
    
    bool writeBufferMuteEx(uint8_t *buffer, uint8_t len);
     
}; //class



#endif //XSPIMASTER_328_H_2018_06_06_10_45_00