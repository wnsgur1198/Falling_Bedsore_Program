#ifndef XSPIMASTER_H_2018_06_03_19_45_00
#define XSPIMASTER_H_2018_06_03_19_45_00
//--------------------------------------------
#include "../x328Util.h"
#include <avr/io.h>

class xSPIMaster
{
    xPort slave;
    uint8_t swap(uint8_t x);

public:
    
    xSPIMaster();
    xSPIMaster(uint8_t preScaler, uint8_t dataMode);
    
    void selectSlave(xPort s)
    {
        slave = s;
    }

    uint8_t selectWrite(uint8_t x);  //startSlave();  receive();  stopSlave();

    uint8_t selectRead()
    {
        return selectWrite(0);
    }

    void write(uint8_t x)
    {
        swap(x);
    }

    uint8_t read()
    {
        return swap(0);
    }

    
    void start()
    {
        slave.put(LOW);
    }

    void stop()
    {
        slave.put(HIGH);
    }
    
    
}; //class
//--------------------------------------------
#endif // XSPIMASTER_H_2018_06_03_19_45_00 