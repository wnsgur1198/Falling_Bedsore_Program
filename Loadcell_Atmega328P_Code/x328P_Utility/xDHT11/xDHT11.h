#ifndef XDHT11_H_2017_07_05_13_08_00
#define XDHT11_H_2017_07_05_13_08_00
//------------------------------------------
#include <avr/io.h>

class xDHT11
{
    static int8_t pinMap[29];
    volatile uint8_t *ddr;     //DDRx
    volatile uint8_t *outPort; //output register
    volatile uint8_t *inPort;  //input register
    uint8_t  bitMask;  //bit mask

    uint8_t data[5];
    bool fetchData();
    bool OK;

public:
    xDHT11(uint8_t pinNo);

    //if (reload == true) call fetchData again;
    uint8_t getTemperature(bool reload=true);  //Integer part of Temperature
    uint8_t getTemperatureFrac();  //Fractional Part of Temperature(Recently measured)

    uint8_t getHumidity(bool reload=true);  //Integer part of Humidity
    uint8_t getHumidityFrac();  //Fractional part of Humidity(Recently measured)

};
//------------------------------------------
#endif // XDHT11_H_2017_07_05_13_08_00 