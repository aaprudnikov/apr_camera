#ifndef MY_NTP_H
#define MY_NTP_H
#include "Arduino.h"

class myNTP {
    private:
    public:
//        myNTP() {};
//        ~myNTP() {};
        bool getTime();
        String getDataTimeShort();
        String getDataTimeLong();
        String getDataTimeSplit();
};
#endif