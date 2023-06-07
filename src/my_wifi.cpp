#include "Arduino.h"
#include "my_wifi.h"
#include "config.h"
#include <WiFi.h>

IPAddress local_ip(192,168,4,22);
IPAddress subnet(255,255,255,0);
const char* dssid = "aprCamera";
const char* dpassword = "!QAZ2wsx";
uint8_t ap_client_count = 0;
bool ap_active_client = false;
String ssidInRange[32];
uint8_t net_count_in_range = 0;
//uint32_t retry_reconnect_tmr;
uint8_t retry_reconnect_cnt = 0;
myWiFi my_WiFi;

// *******************************************************
myWiFi::myWiFi() {
}

// *******************************************************
uint8_t myWiFi::scanNetCountInRange() {
    offline();
    WiFi.mode(WIFI_MODE_STA);
    delay(100);
    WiFi.persistent(false);
    net_count_in_range = WiFi.scanNetworks();

    if (net_count_in_range != 0) {
        for (int i = 0; i < net_count_in_range; ++i) {
            ssidInRange[i] = WiFi.SSID(i);
            debug_println3("my_wifi", "scanNetCountInRange", String(String(i) + ": " + String(ssidInRange[i])).c_str());
        }
    }
    else {
        debug_println3("my_wifi", "scanNetCountInRange", "no networks in range");
//        setMode(WIFI_OFFLINE_MODE);
    }
    return net_count_in_range;
}

// *******************************************************
uint8_t myWiFi::getNetCountInRange() {
    debug_println3("my_wifi", "getNetCountInRange", String(net_count_in_range).c_str());
    return net_count_in_range; 
}

// *******************************************************
String myWiFi::getSsidInRange(int id) {
    return ssidInRange[id];
}
/*
// *******************************************************
void WiFiClientConnected(WiFiEvent_t event, WiFiEventInfo_t info) {
    char connected_mac[18] = "";
    char mac_part[2];
    ap_client_count ++;
    ap_active_client = true;
    for (int i = 0; i < 6; i++) {
        itoa(info.sta_connected.mac[i], mac_part, 16);
        strcat(connected_mac, mac_part);
        if (i < 5) strcat(connected_mac, ":");
    }
    led.flash(3);
    debug_println3("my_wifi", "WiFiClientConnected", String(connected_mac).c_str());
}

// *******************************************************
void WiFiClientDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
    char disconnected_mac[18] = "";
    char mac_part[2];
    ap_client_count --;
    
    if (ap_client_count == 0) {
        ap_active_client = false;
    }

    for (int i = 0; i < 6; i++) {
        itoa(info.sta_connected.mac[i], mac_part, 16);
        strcat(disconnected_mac, mac_part);
        if (i < 5) strcat(disconnected_mac, ":");
    }
    led.flash(2);
    debug_println3("my_wifi", "WiFiClientDisconnected", String(disconnected_mac).c_str());
}
*/
// *******************************************************
void myWiFi::APServer() {
    offline();
    WiFi.softAPConfig(local_ip, local_ip, subnet);
    WiFi.mode(WIFI_MODE_APSTA);
    delay(100);
    WiFi.persistent(false);
    WiFi.softAP(dssid, dpassword);
//    WiFi.onEvent(WiFiClientConnected, SYSTEM_EVENT_AP_STACONNECTED);
//    WiFi.onEvent(WiFiClientDisconnected, SYSTEM_EVENT_AP_STADISCONNECTED);
//    setMode(WIFI_APOINT_MODE);
    debug_println3("my_wifi", "APServer", String(WiFi.softAPIP().toString()).c_str());
}

void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info){
    debug_println3("my_wifi", "WiFiStationConnected", "Ok");
//    my_WiFi.setMode(WIFI_CLIENT_MODE);
    retry_reconnect_cnt = 0;
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info){
    debug_println3("my_wifi", "WiFiStationDisconnected", String("Trying to Reconnect").c_str());
    WiFi.disconnect();
    WiFi.reconnect();
    retry_reconnect_cnt ++;
    if (retry_reconnect_cnt > 10) {
//        my_WiFi.setMode(WIFI_OFFLINE_MODE);
    }
}

// *******************************************************
bool myWiFi::tryConnectToWiFi(const char* ssid, const char* pass) {
    bool m_state = false;
    WiFi.begin(ssid, pass);
    debug_println3("my_wifi", "tryConnectToWiFi", String("ssid: " + String(ssid) + " pass: " + String(pass)).c_str());

    uint32_t try_to_connect_tmr = millis();

    while (millis() - try_to_connect_tmr < 30000 || !m_state) {
        if (WiFi.status() == WL_CONNECTED) {
            Serial.print("\n");
            WiFi.setAutoConnect(true);
            WiFi.persistent(true);
//            WiFi.onEvent(WiFiStationConnected, SYSTEM_EVENT_STA_CONNECTED);
//            WiFi.onEvent(WiFiStationDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);
//            setMode(WIFI_CLIENT_MODE);
            debug_println3("my_wifi", "tryConnectToWiFi", String(WiFi.localIP().toString()).c_str());
            debug_println3("my_wifi", "tryConnectToWiFi", String(WiFi.macAddress()).c_str());
            m_state = true;
            break;
        }
        Serial.print(" . ");
        led.flash(1);
        delay(100);
    }

    if (WiFi.status() != WL_CONNECTED) {
        debug_println3("my_wifi", "tryConnectToWiFi", "false");
        m_state = false;
    }

    return m_state;
}

// *******************************************************
void myWiFi::offline() {
    WiFi.disconnect();
    delay(100);
    WiFi.mode(WIFI_OFF);
    delay(100);
//    setMode(WIFI_OFFLINE_MODE);
//    retry_reconnect_tmr = millis();
    debug_println2("my_wifi", "offline");
}

/*
// *******************************************************
WifiMode myWiFi::getMode() {
    return m_mode;
}

// *******************************************************
void myWiFi::setMode(WifiMode mode) {
    m_mode = mode;
}
*/

// *******************************************************
RetryStatus myWiFi::establishingWiFiConnection() {
    if (WiFi.status() != WL_CONNECTED) {
        debug_println3("my_wifi", "establishingWiFiConnection", "Start");
//        retry_reconnect_tmr = millis();
        if (scanNetCountInRange() != 0) {
            if (sd.checkKnownNetworks()) {
                m_state = RETRY_OK;
            }
            else {
                m_state = RETRY_FAIL;
            }
        }
        else {
            m_state = RETRY_NO_NETWORKS;
        }
    }
    else {
        m_state = RETRY_NOT_NEEDED;
    }
    return m_state;
}

RetryStatus myWiFi:: getState() {
    return m_state;
};