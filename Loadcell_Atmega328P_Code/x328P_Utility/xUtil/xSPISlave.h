#ifndef XSPISLAVE_H_2018_06_04_09_50_00
#define XSPISLAVE_H_2018_06_04_09_50_00
//---------------------------------------------
#include <avr/io.h>

class xSPISlave
{   
    bool interruptDriven;
    void initSlave(bool interruptEnable);
    
public:
 
    xSPISlave();
    xSPISlave(bool interruptEnable);
    
}; //class
//---------------------------------------------
#endif //XSPISLAVE_H_2018_06_04_09_50_00