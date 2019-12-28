/*
 * Loadcell_SPI_Master.cpp
 *
 * Created: 2018-06-14 오전 10:54:49
 *  Author: user
 */ 

#include "../../x328P_Utility/x328Util.h"

#include <avr/io.h>
#include <util/delay.h>
#include <avr/power.h>

#define NUM_SLAVE 4

xUART serial(true);

xSPIMaster_328 spi328;

uint8_t buffer1[17];  //17: 4bytes x 4loadcell + 1checksum byte
uint8_t buffer2[17];
volatile uint8_t *dataBuffer = buffer1;
volatile uint8_t *workBuffer = buffer2;
volatile uint8_t udr;

xPort slave[NUM_SLAVE] = {xPort(23), xPort(24), xPort(25), xPort(26)};
xPort out12Volt(14);

void clearBuffer(uint8_t *buffer, uint8_t len);
void sendData();

ISR(USART_RX_vect)
{
    udr = UDR0;
    if(udr==0xF0) out12Volt.put(HIGH);
    else if(udr==0xFF) out12Volt.put(LOW);
    else sendData();
}
//-----------------------------------
//called from interrupt routine so during this function no other process can be proceeded.
void sendData()  
{
    //if((dataBuffer != buffer1) && (dataBuffer != buffer2)) return;

    serial.transmitBuffer((uint8_t *)dataBuffer, 16);

}

//--------------------------------
void swapBuffer()
{
    volatile uint8_t *temp = dataBuffer;
    dataBuffer = workBuffer;
    workBuffer = temp;
}

//------------------------------------
void clearBuffer(uint8_t *buffer, uint8_t len)
{
    for(uint8_t i=0; i<len; i++) {
        buffer[i] = 0;
    }
}

//------------------------------------
uint8_t getChecksum(uint8_t *buffer, uint8_t len)
{
    uint16_t sum = 0;

    for(uint8_t i=0; i<len; i++) {
        sum += buffer[i];
        if (sum & 0xFF00) sum = (sum & 0x00FF) + 1;
    }

    return (uint8_t)(sum);
}

//----------------------------------------------
int main(void)
{
    if(F_CPU == 8000000UL) clock_prescale_set(clock_div_1);
    
    uint8_t bufferIndex = 0;  

    for(uint8_t i=0; i<NUM_SLAVE; i++)
    {
        slave[i].put(HIGH);
    }

    clearBuffer(buffer1, 17);
    clearBuffer(buffer2, 17);

    out12Volt.put(LOW);

    sei();

    while(1)
    {

        for(uint8_t i=0; i<NUM_SLAVE; i++)
        {
            
            spi328.selectSlave(slave[i]);

            bufferIndex = i * 4;

            //read data from slave inside critical section protected by critical section algorithm.
            spi328.readBufferMuteEx((uint8_t *)(workBuffer+bufferIndex), 4);

            //spi328.blockCount is increased when critical section is blocked by slave.
            //by checking this count you can measure competition between master and slave.
            spi328.blockCount = 0; 
             
            
        }//for

        swapBuffer();
        _delay_us(500);


    }//while
}