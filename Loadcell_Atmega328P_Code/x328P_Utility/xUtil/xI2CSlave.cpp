
#include "../x328Util.h"
 
#include <util/twi.h>
#include <avr/interrupt.h>

uint8_t xI2CSlave::slaveBuffer[SLAVE_BUFFER_LEN];
uint8_t xI2CSlave::bufferIndex=0;

bool  xI2CSlave::firstByte = true;
bool  xI2CSlave::isReady = false;

xI2CSlave::xI2CSlave(uint8_t address)
{
    init(address);  
}

void xI2CSlave::init(uint8_t address)
{
    cli();

    TWAR = address << 1;
    // set the TWCR to enable address matching and enable TWI, clear TWINT, enable TWI interrupt
    TWCR = (1<<TWIE) | (1<<TWEA) | (1<<TWINT) | (1<<TWEN);
    
    isReady = true;
    
    sei();
}

void xI2CSlave::stop(void)
{
    // clear acknowledge and enable bits
    cli();
    TWCR = 0;
    TWAR = 0;
    sei();
}

//void xI2CSlave::getData(uint8_t regaddr, uint8_t *buffer, uint8_t len)
//{
   //for(int i=0; i<len; i++) {
      //buffer[i] = slaveBuffer[regaddr+i];
   //}
//}
//
//void xI2CSlave::setData(uint8_t regaddr, uint8_t *buffer, uint8_t len)
//{
   //for(int i=0; i<len; i++) {
      //slaveBuffer[regaddr+i] = buffer[i];
   //}
//}

void *xI2CSlave::getPtr(uint8_t regaddr)
{
   return (void *)(slaveBuffer + regaddr);
}

void xI2CSlave::eventHandler(void)
{

    switch(TW_STATUS)  //TWSR & 0xF8
    {
        //Master will send some data
        case TW_SR_SLA_ACK:  //0x60
        //case TW_SR_ARB_LOST_SLA_ACK:  //0x68
            firstByte = true;
            TWCR = (1<<TWIE) | (1<<TWINT) | (1<<TWEA) | (1<<TWEN);
            break;
            
        //Slave has received data from Master
        case TW_SR_DATA_ACK:  //0x80
            if(firstByte) {
               bufferIndex = TWDR;
               firstByte = false;
            }  
            else if(bufferIndex < SLAVE_BUFFER_LEN) {
               slaveBuffer[bufferIndex++] = TWDR;          
            }
            else {
               slaveBuffer[SLAVE_BUFFER_LEN - 1] = TWDR;
            }              
                       
            TWCR = (1<<TWIE) | (1<<TWINT) | (1<<TWEA) | (1<<TWEN);
            break;
            
        case TW_ST_SLA_ACK:  //0xA8      
        case TW_ST_DATA_ACK:  //0xB8
            // master is requesting data
            if(bufferIndex < SLAVE_BUFFER_LEN) TWDR = slaveBuffer[bufferIndex++];
            else TWDR =  slaveBuffer[SLAVE_BUFFER_LEN - 1];
            TWCR = (1<<TWIE) | (1<<TWINT) | (1<<TWEA) | (1<<TWEN);
            break;
            
        case TW_BUS_ERROR:  //0x00
            // some sort of erroneous state, prepare TWI to be readdressed
            TWCR = 0;
            TWCR = (1<<TWIE) | (1<<TWINT) | (1<<TWEA) | (1<<TWEN);
            break;
            
        default:
            TWCR = (1<<TWIE) | (1<<TWINT) | (1<<TWEA) | (1<<TWEN);
            
            break;
    }
}

