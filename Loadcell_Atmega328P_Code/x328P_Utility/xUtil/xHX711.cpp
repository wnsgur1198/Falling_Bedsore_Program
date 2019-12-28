
#include "../x328Util.h"

typedef union {
    uint8_t  x[4];
    uint32_t data;
} xData;

xData udata;

xHX711::xHX711(uint8_t dout, uint8_t pd_sck, uint8_t gain)
{
    
    begin(dout, pd_sck, gain);
}

xHX711::xHX711() {}

xHX711::~xHX711() {}

void xHX711::begin(uint8_t dout, uint8_t pd_sck, uint8_t gain)
{
    PD_SCK = xPort(pd_sck);
    DOUT = xPort(dout);

    set_gain(gain);
}

bool xHX711::is_ready()
{
    return (DOUT.get() == LOW);
}

void xHX711::set_gain(uint8_t gain)
{
    switch (gain) {
        case 128:		// channel A, gain factor 128
        GAIN = 1;
        break;
        case 64:		// channel A, gain factor 64
        GAIN = 3;
        break;
        case 32:		// channel B, gain factor 32
        GAIN = 2;
        break;
    }

    //digitalWrite(PD_SCK, LOW);
    PD_SCK.put(LOW);
    read();
}

uint8_t xHX711::shiftIn(uint8_t dir)
{

    uint8_t shift, data = 0;

    for(uint8_t i=0; i<8; i++) {
        
        shift = i;

        if(dir == MSBFIRST) shift = 7-i;
        
        PD_SCK.put(HIGH);
        data |= (DOUT.get() << shift);
        PD_SCK.put(LOW);

    }

    return data;

}

int32_t xHX711::read()
{
    // wait for the chip to become ready
    while (!is_ready()) ;

    // pulse the clock pin 24 times to read the data
    udata.x[2] = shiftIn();
    udata.x[1] = shiftIn();
    udata.x[0] = shiftIn();

    // set the channel and the gain factor for the next reading using the clock pin
    for (unsigned int i = 0; i < GAIN; i++) {
        PD_SCK.put(HIGH);
        PD_SCK.put(LOW);
    }


    if (udata.x[2] & 0x80) {
        udata.x[3] = 0xFF;
    }
    else {
        udata.x[3] = 0x00;
    }

    return (int32_t)(udata.data);
}

int32_t xHX711::read_average(uint8_t times)
{
    int32_t sum = 0;
    for (uint8_t i = 0; i < times; i++) {
        sum += read();
        //yield();
    }
    return sum / times;
}

void xHX711::power_down()
{

    PD_SCK.put(LOW);
    PD_SCK.put(HIGH);
}

void xHX711::power_up()
{
    PD_SCK.put(LOW);
}