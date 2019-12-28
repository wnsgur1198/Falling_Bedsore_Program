#include "../x328Util.h"
#include <avr/io.h>
#include <util/delay.h>


xSPISlave_328::xSPISlave_328()
{
    spi = xSPISlave();
    initValues();
}

void xSPISlave_328::initValues()
{
    dataLen=0;
    cmd = SPI_CMD_NULL;
    bufferIndex = 0;
    slaveIn = false;
    masterIn = false;
    turn = SPI_MASTER;
    currentMode = SPI_MODE_NULL;
    dataReady = false;
    bufferEmpty = true;
    blockCount = 0;
    for(uint8_t i=0; i<SPI_SLAVE_BUFFER_LEN; i++) {
        bufferIn[i] = 0;
        bufferOut[i] = 0;
    }
}

void xSPISlave_328::interruptHandler()
{
    cmd = SPDR;

    if(currentMode == SPI_MODE_NULL) {
        switch(cmd)
        {
            case SPI_CMD_NULL:
                SPDR = SPI_CMD_NULL;
                break;
            
            case SPI_CMD_SLAVE_DATA_READY:
                SPDR = (dataReady) ? 1 : 0;
                break;

            case SPI_CMD_SLAVE_BUFFER_READY:
                SPDR = (bufferEmpty) ? 1 : 0;
                break;

            case SPI_CMD_CS_WANT:
                masterIn = true;
                SPDR = SPI_CMD_NULL;
                break;

            case SPI_CMD_CS_WAIT:
                turn = SPI_SLAVE;
                SPDR = SPI_CMD_NULL;
                break;

            case SPI_CMD_CS_IN:
                SPDR = ((slaveIn) && (turn == SPI_SLAVE)) ? CS_BUSY : CS_EMPTY;
                break;

            case SPI_CMD_CS_OUT:
                masterIn = false;
                SPDR = SPI_CMD_NULL;
                break;

            case SPI_CMD_DATA_LENGTH:
                currentMode = SPI_MODE_LENGTH_READING;
                SPDR = SPI_CMD_NULL;
                break;
            
            case SPI_CMD_READ_BEGIN:
            
                if(dataLen > 0) {
                    currentMode = SPI_MODE_MASTER_READING;
                    SPDR = bufferOut[bufferIndex=0];
                }
                else {
                    SPDR = SPI_CMD_ERROR;
                }
                break;
            
            case SPI_CMD_WRITE_BEGIN:
                if(dataLen > 0) {
                    currentMode = SPI_MODE_MASTER_WRITING;
                    bufferIndex=0;
                }
                else {
                    SPDR = SPI_CMD_ERROR;
                }
                break;

        }
    }//if SPI_MODE_NULL

    else {
        switch(currentMode) {
            case SPI_MODE_LENGTH_READING:
                dataLen = SPDR;
                currentMode = SPI_MODE_NULL;
                SPDR = SPI_CMD_NULL;
                break;

            case SPI_MODE_MASTER_READING:
                bufferIndex++;
                if(bufferIndex < dataLen) {
                    SPDR = bufferOut[bufferIndex];
                }
                else {
                    SPDR = bufferOut[dataLen]; //checksum follows after data
                    dataReady = false;
                    currentMode = SPI_MODE_NULL;
                }
                break;

            case SPI_MODE_MASTER_WRITING:
            
                if(bufferIndex < dataLen) {
                    bufferIn[bufferIndex] = SPDR;
                    bufferIndex++;
                }
                else {
                    bufferIn[dataLen] = SPDR; //checksum follows after data
                    bufferEmpty = false;
                    currentMode = SPI_MODE_NULL;
                }
            
                break;
        }//switch
    }//else if some special Mode

}

void xSPISlave_328::enterCriticalSection()
{
    slaveIn = true;
    turn = SPI_MASTER;  //masterIn
    while((masterIn) && (turn==SPI_MASTER)) {
        blockCount++;
        //_delay_ms(1); //master want to go into criticalSection and it is master's turn wait..
        _delay_us(250);
    }
}

void xSPISlave_328::leaveCriticalSection()
{
    slaveIn = false;
}

bool xSPISlave_328::readeBufferMuteEx(uint8_t *buffer, uint8_t len)
{
    
    if(bufferEmpty) return false;

    enterCriticalSection();

    for(uint8_t i=0; i<=len; i++) {  //checksum follows after last data byte
        buffer[i] = bufferIn[i];
    }

    bufferEmpty = true;

    leaveCriticalSection();
    
    return true;
}

bool xSPISlave_328::writeBufferMuteEx(uint8_t *buffer, uint8_t len)
{
    enterCriticalSection();

    for(uint8_t i=0; i<=len; i++) { //checksum follows after last data byte
        bufferOut[i] = buffer[i];
    }

    dataReady = true;

    leaveCriticalSection();

    return true;
}