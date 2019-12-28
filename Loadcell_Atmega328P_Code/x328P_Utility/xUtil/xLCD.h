
#ifndef XLCD_H_2017_01_27_18_45_58
#define XLCD_H_2017_01_27_18_45_58
//-------------------------------------------------

#include <avr/io.h>
#include "xPort.h"

// LCD instructions
#define LCD_CLEAR 0x01
#define LCD_HOME  0x02

#define LCD_OUTPUT_LEFT   0x04
#define LCD_OUTPUT_RIGHT  0x06
//#define LCD_SHIFT_RIGHT 0x05
//#define LCD_SHIFT_LEFT  0x07

#define LCD_DISPLAY_OFF_CURSOR_OFF  0x08
#define LCD_DISPLAY_ON_CURSOR_OFF   0x0C
#define LCD_DISPLAY_ON_CURSOR_ON    0x0E
#define LCD_DISPLAY_ON_CURSOR_BLINK 0x0F

#define LCD_CURSOR_LEFT   0x10
#define LCD_CURSOR_RIGHT  0x14
#define LCD_DISPLAY_SHIFT_LEFT  0x18
#define LCD_DISPLAY_SHIFT_RIGHT 0x1C

#define LCD_8_1_5X7_MODE  0x30
#define LCD_8_2_5X7_MODE  0x38
#define LCD_4_1_5X7_MODE  0x20
#define LCD_4_2_5X7_MODE  0x28

#define LCD_LINE_1_HOME 0x80  // set cursor position start of line 1
#define LCD_LINE_2_HOME 0xC0  // set cursor position start of line 2

class xLCD
{
   xPort rs, en, d4, d5, d6, d7;
   void put4bits(uint8_t x);
   
public:
   xLCD(uint8_t rs_pin, uint8_t en_pin, uint8_t d4_pin, uint8_t d5_pin, uint8_t d6_pin, uint8_t d7_pin);
   void control(uint8_t x);
   void put(char x);
   void put(int8_t x);
   void put(int16_t x);
   void put(int32_t x);
   void put(uint8_t x);
   void put(uint16_t x);
   void put(uint32_t x);
   void put(float x);
   void put(double x);
   void put(const char *x);
   void gotoXY(uint8_t x, uint8_t y);
   void clear();
};
//--------------------------------------
#endif /* XLCD_H_2017_01_27_18_45_58 */