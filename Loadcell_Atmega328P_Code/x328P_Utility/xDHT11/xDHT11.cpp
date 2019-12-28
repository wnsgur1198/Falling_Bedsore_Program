
#include "../x328Util.h"
#include <avr/io.h>
#include <util/delay.h>

//--------------------------XX C6 D0 D1 D2 D3 D4 +5 GN B6 B7 D5 D6 D7 B0 B1 B2 B3 B4 B5 AV AR GN C0 C1 C2 C3 C4 C5
//--------------------------XX  1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28
int8_t xDHT11::pinMap[] = {-1, 6, 0, 1 ,2, 3, 4,-1,-1, 6, 7, 5, 6, 7, 0, 1, 2, 3, 4, 5,-1,-1,-1, 0, 1, 2, 3, 4, 5};

xDHT11::xDHT11(uint8_t pinNo)
{
    OK = false;

    if((pinNo >= 2 && pinNo <= 6) || (pinNo >= 11 && pinNo <= 13)) {
        ddr = &DDRD;
        outPort = &PORTD;
        inPort = &PIND;
        bitMask = 1 << pinMap[pinNo];
    }
    else if((pinNo >= 9 && pinNo <= 10) || (pinNo >= 14 && pinNo <= 19)) {
        ddr = &DDRB;
        outPort = &PORTB;
        inPort = &PINB;
        bitMask = 1 << pinMap[pinNo];
    }
    else if((pinNo ==1) || (pinNo >= 23 && pinNo <= 28)) {
        ddr = &DDRC;
        outPort = &PORTC;
        inPort = &PINC;
        bitMask = 1 << pinMap[pinNo];
    }
    else {
        ddr = 0;
        outPort = 0;
        inPort = 0;
        bitMask = 0;
    }

    // DHT11's datasheet says that device needs at least 1 second waiting for its unstable state period.  
    // So we wait 2 secs!!
    _delay_ms(2000);
}


bool xDHT11::fetchData()
{
    uint8_t cnt, check;
    int8_t i,j;
    
    //--- Sensor communication start --------------------
    

    // DHT pin을 출력으로 세트
    *ddr |= bitMask;
   
    //-- millisecond 동안 기다려야 하므로 카운터의 속도를 현재 시스템 클럭 스피드의 1024분의 1로 줄임
    //(TCNT0 레지스터의 길이가 8비트로 0부터 255까지만 카운트 할 수 있으므로 너무 빨리 카운트하면 overflow발생) --

    TCCR0B = 0x05;
    
    TCNT0 = 0;
    
    //-- DHT 핀에 Low값을 출력 ---
    *outPort &= ~bitMask;
    
    //-- 20msec동안 대기 --(현재 클럭 스피드로 20msec는 160 카운트와 같음(255 범위에 들어옴)
    while(TCNT0 < 160);
    
    // 이후로 측정하는 시간은 마이크로 초 단위 이므로 카운터의 속도를 현재 클럭의 1/8로 지정
    //그래도 충분히 8비트의 범위에 들어옴(이제부터 1카운터 =  1마이크로 초)

    TCCR0B = 0x02;
    
    TCNT0 = 0;
    
    //-- DHT 핀에 High값을 출력---
    *outPort |= bitMask;
    
    //-- DHT가 연결된 핀을 입력 핀으로 바꿈 ----
    *ddr &= ~bitMask;
    
    //-- 출력 핀의 값이 Low로 바뀔 때까지 대기(20~40usec 범위가 정상) -----
    while((*inPort) & bitMask) {
        if (TCNT0 >= 60) return false;
    }
    
    // --  Sensor preamble -------------------------
    
    TCNT0 = 0;
    
    //-- DHT 핀의 값이 High로 바뀔 때까지 대기 (80us 정도가 정상) ---
    while(!((*inPort) & bitMask)) {
        if (TCNT0 >= 100) return false;
    }
    
    TCNT0 = 0;
    
    //-- DHT 핀의 값이 다시 Low로 바뀔 때까지 대기 (80us 정도가 정상) -----------
    while((*inPort) & bitMask) {
        if (TCNT0 >= 100) return false;
    }
    
    //--- 이제부터 데이터를 한 비트씩 40비트(5바이트) 읽어들임 -----------------------   
    for (i = 0; i < 5; ++i)
    {
        for(j = 7; j >= 0; --j)
        {
            TCNT0 = 0;
            
            //-- 먼저 50usec 동안 Low 값이 지속된 후
            while(!((*inPort) & bitMask)) { 
                if (TCNT0 >= 70) return false; 
            }
            
            TCNT0 = 0;
            
            
            //-- DHT 핀의 값이 Low로 변할 때까지 대기 ----- 
            //-- 한 동안 High상태가 지속되는데 이 시간이 26~28use이면 비트 값은 0을 의미하고
            //   High상태가 약 70usec 정도 지속되면 비트 값은 1을 의미함.
            
            while((*inPort) & bitMask) { 
                if (TCNT0 >= 90) return false; 
            }

            //DHT핀의 값이 Low로 떨어진 후 TCNT0의 값을 체크하여 High가 지속된 시간을 알아낸 후 0 혹은 1을 판정함.
            cnt = TCNT0;
            
            if (cnt >= 20 && cnt <= 35) { 
                data[i] &= ~(1<<j);  //0
            }
            else if (cnt >= 60 && cnt <= 80) { 
                data[i] |= (1<<j);  //1
            }
            else {
                return false;
            }
        }
    }
    
    //-- 40비트의 데이터를 읽은 후 checksum 값 비교 ------
    //앞의 4바이트의 값을 모두 더하여 256으로 나눈 나머지가 마지막 다섯 번 째 바이트의 값과 같으면 데이터는 valid함.
    
    check = (data[0] + data[1] + data[2] + data[3]) & 0xFF;
    
    if (check != data[4]) return false;
    
    return true;

}//fetchData2()

uint8_t xDHT11::getHumidity(bool reload)  //Integer part of Humidity
{
    if(reload) OK=fetchData();
    if(!OK) return 0xff;
    return data[0];
}

uint8_t xDHT11::getHumidityFrac()  //Fractional part of Humidity(Recently measured)
{
    if(!OK) return 0xff;
    return data[1];
}

uint8_t xDHT11::getTemperature(bool reload)  //Integer part of Temperature
{
   
   if(reload) OK=fetchData();
   if(!OK) return 0xff;
   return data[2];

}

uint8_t xDHT11::getTemperatureFrac()  //Fractional Part of Temperature(Recently measured)
{
    if(!OK) return 0xff;
    return data[3];
}


