
#ifndef XI2CSLAVE_H_2017_07_13_11_49_00
#define XI2CSLAVE_H_2017_07_13_11_49_00
//--------------------------------------
#include <avr/interrupt.h>
#include <stdint.h>

#define SLAVE_BUFFER_LEN 32

ISR(TWI_vect);


class xI2CSlave
{
    static uint8_t slaveBuffer[SLAVE_BUFFER_LEN];
    static uint8_t bufferIndex;
    
    static bool firstByte;
    static bool isReady;
    
public:
    
    
    xI2CSlave(uint8_t address);
    
    static void init(uint8_t address);

    static void stop(void);

    static void eventHandler(void);
    
    //static void getData(uint8_t regaddr, uint8_t *buffer, uint8_t len);
    //
    //static void setData(uint8_t regaddr, uint8_t *buffer, uint8_t len);

    static void *getPtr(uint8_t regaddr);

};
//--------------------------------------
#endif /* XI2CSLAVE_H_2017_07_13_11_49_00 */