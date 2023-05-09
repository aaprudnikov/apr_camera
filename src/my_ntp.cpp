#include "my_ntp.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include "config.h"

#define TAG "myNTP"

bool ntp_awailable = false;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "0.ru.pool.ntp.org", 10800, 60000);


// *******************************************************
bool myNTP::getTime() {
    timeClient.begin();
    timeClient.setTimeOffset(10800);
    uint8_t try_count = 0;

    while(!timeClient.update() && try_count < 3) {
        try_count++;
        debug_println3("my_ntp", "getTime", String("attempt: " + String(try_count)).c_str());
        delay(500);
    }

    if (try_count > 3) {
        ntp_awailable = false;
        debug_println3("my_ntp", "getTime", "false")
        return ntp_awailable;
    }

    ntp_awailable = true;
    debug_println3("my_ntp", "getTime", String(timeClient.getFormattedTime()).c_str())
    return ntp_awailable;
}

// *******************************************************
String myNTP::getDataTimeShort() {
    String current_data_time = "";
    char buff_current_data_time[32];
    unsigned long current_time;

    if (!ntp_awailable) {
        current_data_time = "2022-01-01";
    }
    else {
        current_time = timeClient.getEpochTime();
        sprintf(buff_current_data_time, "%02d-%02d-%02d", year(current_time), month(current_time), day(current_time));
        current_data_time = String(buff_current_data_time);
    }
    return current_data_time;
}

// *******************************************************
String myNTP::getDataTimeLong() {
    String current_data_time = "";
    char buff_current_data_time[32];
    unsigned long current_time;

    if (!ntp_awailable) {
        current_data_time = "2022-01-01 00:00:00";
    }
    else {
        current_time = timeClient.getEpochTime();
        sprintf(buff_current_data_time, "%02d-%02d-%02d %02d:%02d:%02d", year(current_time), month(current_time), day(current_time), hour(current_time), minute(current_time), second(current_time));
        current_data_time = String(buff_current_data_time);
    }
    return current_data_time;
}

// *******************************************************
String myNTP::getDataTimeSplit() {
    String current_data_time = "";
    char buff_current_data_time[32];
    unsigned long current_time;

/*
    if (!ntp_awailable) {
        current_data_time = "20220101000000";
    }
    else {
        current_time = timeClient.getEpochTime();
        sprintf(buff_current_data_time, "%02d%02d%02d%02d%02d%02d", year(current_time), month(current_time), day(current_time), hour(current_time), minute(current_time), second(current_time));
        current_data_time = String(buff_current_data_time);
    }
*/
        current_time = timeClient.getEpochTime();
        sprintf(buff_current_data_time, "%02d%02d%02d%02d%02d%02d", year(current_time), month(current_time), day(current_time), hour(current_time), minute(current_time), second(current_time));
        current_data_time = String(buff_current_data_time);

    return current_data_time;
}