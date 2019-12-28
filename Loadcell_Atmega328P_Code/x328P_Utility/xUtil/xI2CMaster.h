
#ifndef XI2CMASTER_H_2017_07_12_16_30_00
#define XI2CMASTER_H_2017_07_12_16_30_00
//-------------------------------------------
#include <avr/io.h>

#define I2C_READ  0x01
#define I2C_WRITE 0x00

class xI2CMaster 
{
    static bool isInitialized;
    static bool start(uint8_t address);
    static bool write(uint8_t data);
    static uint8_t read_ack(void);
    static uint8_t read_nack(void);
    static void stop(void);
    
public:
    static int32_t errorCode;

    xI2CMaster();
    xI2CMaster(uint32_t);
    xI2CMaster(uint32_t, uint8_t);
     
    static void init(uint32_t, uint8_t);

    static bool setErrorCode(uint8_t errcd)
    {
        errorCode = errorCode * 10 + errcd;
        return false;
    }
     
    static bool writeBytes(uint8_t address, uint8_t regaddr, uint8_t* data, uint16_t length);
    static bool readBytes(uint8_t address, uint8_t regaddr, uint8_t* data, uint16_t length);  

    static bool writeBits(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t data);
    static bool writeBitsW(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint16_t data);

    static int8_t readBits(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length);
    static uint16_t readBitsW(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length); 
};
//-------------------------------------------
#endif /* XI2CMASTER_H_2017_07_12_16_30_00 */