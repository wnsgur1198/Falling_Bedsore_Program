#ifndef XSPISLAVE_328_H_2018_06_04_09_50_00
#define XSPISLAVE_328_H_2018_06_04_09_50_00
//---------------------------------------------

#include <avr/io.h>

#include "xSPISlave.h"

#define SPI_SLAVE  (0x00)
#define SPI_MASTER (0x01)

#define SPI_MODE_NULL (0x00)
#define SPI_MODE_LENGTH_READING (0x01)
#define SPI_MODE_MASTER_READING (0x02)
#define SPI_MODE_MASTER_WRITING (0x03)

#define SPI_SLAVE_BUFFER_LEN (10)

class xSPISlave_328
{
    xSPISlave spi;
    
    volatile uint8_t dataLen;
    volatile uint8_t cmd;
    volatile uint8_t bufferIndex;
    volatile bool slaveIn;
    volatile bool masterIn;
    volatile uint8_t turn;
    volatile uint8_t currentMode;  
      
public:

    xSPISlave_328();
    
    void initValues();
    
    bool dataReady;   // output data
    bool bufferEmpty; //input buffer
    uint8_t bufferIn[SPI_SLAVE_BUFFER_LEN];
    uint8_t bufferOut[SPI_SLAVE_BUFFER_LEN];
    uint32_t blockCount;
    
    void interruptHandler();
    bool readeBufferMuteEx(uint8_t *buffer, uint8_t len);
    bool writeBufferMuteEx(uint8_t *buffer, uint8_t len);
    void enterCriticalSection();
    void leaveCriticalSection();
    
}; //class
//---------------------------------------------
#endif //XSPISLAVE_328_H_2018_06_04_09_50_00 