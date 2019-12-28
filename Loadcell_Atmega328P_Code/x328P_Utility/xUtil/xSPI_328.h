#ifndef XSPI_328_H_2018_06_03_19_45_00
#define XSPI_328_H_2018_06_03_19_45_00
//-----------------------------------

#define SPI_CMD_NULL  (0x00)
#define SPI_CMD_ERROR (0xFF)

#define SPI_CMD_CS_WANT (0xA0)  //Critical Section Commands
#define SPI_CMD_CS_WAIT (0xA1)
#define SPI_CMD_CS_IN   (0xA2)
#define SPI_CMD_CS_OUT  (0xA3)

#define SPI_CMD_SLAVE_DATA_READY   (0xB0)
#define SPI_CMD_SLAVE_BUFFER_READY (0xB1)
#define SPI_CMD_DATA_LENGTH        (0xB2)

#define SPI_CMD_READ_BEGIN  (0xC0)
//#define SPI_CMD_READ_END    (0xC1)

#define SPI_CMD_WRITE_BEGIN (0xD0)
//#define SPI_CMD_WRITE_END   (0xD1)


//Critical Section states values
#define CS_BUSY  (0x00)
#define CS_EMPTY (0x01)


//------------------------------------
#endif // XSPI_328_H_2018_06_03_19_45_00