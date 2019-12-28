
#include "../x328Util.h"
#include <avr/io.h>
#include <util/twi.h>

bool xI2CMaster::isInitialized = false;
int32_t xI2CMaster::errorCode = 0;

//--------------------------------
xI2CMaster::xI2CMaster()
{
    init(400000UL, 1);
}

//--------------------------------
xI2CMaster::xI2CMaster(uint32_t fscl)
{
    init(fscl, 1);
}

//--------------------------------
xI2CMaster::xI2CMaster(uint32_t fscl, uint8_t prescaler)
{
    init(fscl, prescaler);
}

//--------------------------------
void xI2CMaster::init(uint32_t fscl, uint8_t prescaler)
{
    if(isInitialized) return;
    //TWBR = (uint8_t)TWBR_val;
    TWBR = (uint8_t)(((F_CPU / fscl) - 16 ) / (2 * prescaler));
    isInitialized = true;
}

//--------------------------------
bool xI2CMaster::start(uint8_t address)
{
    errorCode = 0;
    // reset TWI control register
    TWCR = 0;
    // transmit START condition
    TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
    // wait for end of transmission
    while( !(TWCR & (1<<TWINT)) );
     
    // check if the start condition was successfully transmitted
    if(TW_STATUS != TW_START) return setErrorCode(1);
     
    // load slave address into data register
    TWDR = address;
    // start transmission of address
    TWCR = (1<<TWINT) | (1<<TWEN);
    // wait for end of transmission
    while( !(TWCR & (1<<TWINT)) );
     
    // check if the device has acknowledged the READ / WRITE mode
    if ( (TW_STATUS != TW_MT_SLA_ACK) && (TW_STATUS != TW_MR_SLA_ACK) ) return setErrorCode(2);
     
    return true;
}

//--------------------------------
void xI2CMaster::stop(void)
{
    // transmit STOP condition
    TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
}

//--------------------------------
bool xI2CMaster::write(uint8_t data)
{
    // load data into data register
    TWDR = data;
    // start transmission of data
    TWCR = (1<<TWINT) | (1<<TWEN);
    // wait for end of transmission
    while( !(TWCR & (1<<TWINT)) );
     
    if( TW_STATUS != TW_MT_DATA_ACK ) return setErrorCode(1);
     
    return true;
}

//--------------------------------
uint8_t xI2CMaster::read_ack(void)
{
    // start TWI module and acknowledge data after reception
    TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
    // wait for end of transmission
    while( !(TWCR & (1<<TWINT)) );
    // return received data from TWDR
    return TWDR;
}

//--------------------------------
uint8_t xI2CMaster::read_nack(void)
{
    // start receiving without acknowledging reception
    TWCR = (1<<TWINT) | (1<<TWEN);
    // wait for end of transmission
    while( !(TWCR & (1<<TWINT)) );
    // return received data from TWDR
    return TWDR;
}

//--------------------------------
bool xI2CMaster::writeBytes(uint8_t devaddr, uint8_t regaddr, uint8_t* data, uint16_t length)
{

    if(!isInitialized) return false;

    if(!start((devaddr << 1) | I2C_WRITE)) return setErrorCode(1);
    
    if(!write(regaddr)) return setErrorCode(2);
    

    for (uint16_t i = 0; i < length; i++)
    {
        if(!write(data[i])) return setErrorCode(3);
    }
     
    stop();
     
    return true;
}

//--------------------------------
bool xI2CMaster::readBytes(uint8_t devaddr, uint8_t regaddr, uint8_t *data, uint16_t length)
{

    if(!isInitialized) return false;
    
    if(!start((devaddr << 1) | I2C_WRITE)) return setErrorCode(1);

    if(!write(regaddr)) return setErrorCode(2);

    if(!start((devaddr << 1) | I2C_READ)) return setErrorCode(3);
    
    for (uint16_t i = 0; i < (length-1); i++)
    {
        data[i] = read_ack();
    }
    data[(length-1)] = read_nack();
     
    stop();
     
    return true;
}

//--------------------------------------------------
//devAddr Slave의 redAddr 레지스터의 bitStart 위치에서부터 length만큼의 길이의 비트를 data로 대체(data는 오른쪽 aligned 된 값)
bool xI2CMaster::writeBits(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t data)
{

    uint8_t b;
    if(!readBytes(devAddr, regAddr, &b, 1)) return setErrorCode(1);
    
    uint8_t mask = ((1 << length) - 1) << (bitStart - length + 1);
    data <<= (bitStart - length + 1); // shift data into correct position
    data &= mask; // zero all non-important bits in data
    b &= ~(mask); // zero all important bits in existing byte
    b |= data; // combine data with existing byte
    if(!writeBytes(devAddr, regAddr, &b, 1)) return setErrorCode(2);

    return true;
}

//-------------------------------------------------------------
bool xI2CMaster::writeBitsW(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint16_t data) 
{
    uint16_t w;
    if (!readBytes(devAddr, regAddr, (uint8_t *)(&w), sizeof(uint16_t))) return setErrorCode(1);

    uint16_t mask = ((1 << length) - 1) << (bitStart - length + 1);
    data <<= (bitStart - length + 1); // shift data into correct position
    data &= mask; // zero all non-important bits in data
    w &= ~(mask); // zero all important bits in existing word
    w |= data; // combine data with existing word
    if(!writeBytes(devAddr, regAddr, (uint8_t *)(&w), sizeof(uint16_t))) return setErrorCode(2);

    return true;

}

//------------------------------------------------------
int8_t xI2CMaster::readBits(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length) 
{

    uint8_t b;
    if(!readBytes(devAddr, regAddr, &b, 1)) return setErrorCode(1);

    uint8_t mask = ((1 << length) - 1) << (bitStart - length + 1);
    b &= mask;
    b >>= (bitStart - length + 1);

    return b;
}

//----------------------------------------------------------
uint16_t xI2CMaster::readBitsW(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length) 
{
    uint16_t w;
    if(!readBytes(devAddr, regAddr, (uint8_t *)(&w), sizeof(uint16_t))) return setErrorCode(1);
    
    uint16_t mask = ((1 << length) - 1) << (bitStart - length + 1);
    w &= mask;
    w >>= (bitStart - length + 1);
    return w;
}