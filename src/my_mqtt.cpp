#include "my_mqtt.h"
#include "ThingSpeak.h"
#include "my_wifi.h"
#include <WiFi.h>
#include "config.h"

unsigned long myChannelNumber = 2174842;
const char * MQTT_API_KEY = "ZZKU02UA0WVWBJA5";

struct UptimeForRetry {
    int value;
    bool flag;
} uptimeRetry;

struct CaptureForRetry {
    int value;
    bool flag;
} captureRetry;

WiFiClient  client;

void myMQTT::sendData(PublishData data_type, int value) {
    String data_type_str;
    uint8_t field;
    switch (data_type) {
        case MY_UPTIME:
            data_type_str = "MY_UPTIME";
            field = 1;
            break;
        
        case MY_CAPTURE_CNT:
            data_type_str = "MY_CAPTURE_CNT";
            field = 2;
            break;
    }

    if (my_wifi.getState() != RETRY_NOT_NEEDED) {
        debug_println3("my_mqtt", String("sendData: " + data_type_str).c_str(), "internet_fail");
        return;
    }

    ThingSpeak.begin(client);
    ThingSpeak.setField(field, value);
    int resp = ThingSpeak.writeFields(myChannelNumber, MQTT_API_KEY);
    if(resp == 200) {
        debug_println3("my_mqtt", String("sendData: " + data_type_str).c_str(), String("value: " + String(value) + " Channel update successful").c_str());
        if (data_type == MY_UPTIME) uptimeRetry.flag = false;
        if (data_type == MY_CAPTURE_CNT) captureRetry.flag = false;
    }
    else{
        debug_println3("my_mqtt", String("sendData: " + data_type_str).c_str(), String("Problem updating channel. HTTP error code: " + String(resp)).c_str());
        if (data_type == MY_UPTIME) {
            uptimeRetry.flag = true;
            uptimeRetry.value = value;
        }
        if (data_type == MY_CAPTURE_CNT) {
            captureRetry.flag = true;
            captureRetry.value = value;
        }
    }
}

void myMQTT::retringSendData() {
    static uint32_t retry_cnt;
    
    if (millis() - retry_cnt < 15000) {
        return;
    }

    retry_cnt = millis();

    if (uptimeRetry.flag) {
        sendData(MY_UPTIME, uptimeRetry.value);
        debug_println3("my_mqtt", "retringSendData", "MY_UPTIME");
        return;
    }

    if (captureRetry.flag) {
        sendData(MY_CAPTURE_CNT, captureRetry.value);
        debug_println3("my_mqtt", "retringSendData", "MY_CAPTURE_CNT");
        return;
    }
}