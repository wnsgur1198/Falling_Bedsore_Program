/*
 * xUtil.h
 *
 * Created: 2017-01-29 오후 1:41:43
 *  Author: Owner
 */ 
//----------------------------------------

#ifndef X328UTIL_H_2017_01_29_13_41_43
#define X328UTIL_H_2017_01_29_13_41_43

//-------------------------------------------------
#ifndef __AVR_ATmega328P__
#error "ATmega328P에만 적용되는 라이브러리입니다."
#endif
//--------------------------------------------------
#ifdef F_CPU
#error "F_CPU is already defined.  x328_Utility library must be rebuilt if F_CPU value is different from one which is used when Library is built(Check it..)"
#endif

#ifdef BAUD
#error "BAUD is already defined.  x328_Utility  library must be rebuilt if BAUD value is different from the one which is used when Library is built(Check it..)"
#endif

//x328_Utility 라이브러리는 다음의 F_CPU, BAUD 두 값을 사용하여 빌드되었습니다.  
//만약 아래의 두 값이 프로그램이 사용하려는 값과 다르다면 아래의 값을 수정하고 라이브러리를 다시빌드해야 합니다.
#define F_CPU 8000000UL
#define BAUD 9600UL

#define HIGH 1
#define LOW  0

#include "xUtil/xUnion.h"
#include "xUtil/xI2CMaster.h"
#include "xUtil/xI2CSlave.h"
#include "xUtil/xPort.h"
#include "xUtil/xUART.h"
#include "xUtil/xLCD.h"
#include "xUtil/xADC.h"
#include "xUtil/xBeep.h"
#include "xUtil/xSPIMaster.h"
#include "xUtil/xSPISlave.h"
#include "xUtil/xSPI_328.h"
#include "xUtil/xSPIMaster_328.h"
#include "xUtil/xSPISlave_328.h"
#include "xUtil/xHX711.h"
#include "xDHT11/xDHT11.h"


//----------------------------------------
#endif /* X328UTIL_H_2017_01_29_13_41_43 */