
#ifndef XUNION_H_2017_07_19_10_42_00
#define XUNION_H_2017_07_19_10_42_00
//-----------------------------------------

#define BIG_ENDIAN 1
#define LITTLE_ENDIAN 0

#include <avr/io.h>

typedef union {
    float f;
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    uint8_t buffer[10];
} xUnion;

//-----------------------------------------
#endif /* XUNION_H_2017_07_19_10_42_00 */