#include "../x328Util.h"
#include <avr/io.h>
#include <util/delay.h>

xSPIMaster_328::xSPIMaster_328(uint8_t preScaler, uint8_t dataMode)
{
    spi = xSPIMaster(preScaler, dataMode);

    blockCount = 0;
}

void xSPIMaster_328::readBuffer(uint8_t *buffer, uint8_t len)
{
    for(uint8_t i=0; i<len; i++) {
        buffer[i] = spi.selectRead();
    }
}

void xSPIMaster_328::writeBuffer(uint8_t *buffer, uint8_t len)
{
    for(uint8_t i=0; i<len; i++) {
        spi.selectWrite(buffer[i]);
    }
}


bool xSPIMaster_328::readBufferMuteEx(uint8_t *buffer, uint8_t len)
{
    //if there is no data written since last access then leave buffer unchanged.(old data)

    sendCommand(SPI_CMD_SLAVE_DATA_READY);
    if(spi.selectRead() == 0) return false; //old data
    
    sendCommand(SPI_CMD_DATA_LENGTH);
    spi.selectWrite(len);
    
    enterCriticalSection();
    
    sendCommand(SPI_CMD_READ_BEGIN);

    readBuffer(buffer, len);

    buffer[len] = spi.selectRead(); //checksum: buffer should have one more room(byte) for checksum;
    
    leaveCriticalSection();
    
    return true; //new data
}

bool xSPIMaster_328::writeBufferMuteEx(uint8_t *buffer, uint8_t len)
{
    sendCommand(SPI_CMD_SLAVE_BUFFER_READY);
    if(spi.selectRead() == 0) return false; //data not read by slave
    
    sendCommand(SPI_CMD_DATA_LENGTH);
    spi.selectWrite(len);
    
    enterCriticalSection();
    
    sendCommand(SPI_CMD_WRITE_BEGIN);
    
    writeBuffer(buffer, len);
    
    spi.selectWrite(~buffer[len]);  //checksum: buffer should have one more room(byte) for checksum
    
    leaveCriticalSection();

    return true;
}

void xSPIMaster_328::enterCriticalSection()
{
    sendCommand(SPI_CMD_CS_WANT);

    sendCommand(SPI_CMD_CS_WAIT);
    
    for(;;) {
        
        sendCommand(SPI_CMD_CS_IN);

        if(spi.selectRead() == CS_EMPTY) break;

        blockCount++;
        _delay_us(250);
    }
}

void xSPIMaster_328::leaveCriticalSection()
{
    sendCommand(SPI_CMD_CS_OUT);
}

