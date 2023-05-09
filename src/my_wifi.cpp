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

// *******************************************************
myWiFi::myWiFi() {
}


// *******************************************************
void myWiFi::clientMode() {
    offline();
    WiFi.mode(WIFI_MODE_STA);
    delay(100);
    WiFi.persistent(false);
    m_mode = WIFI_CLIENT_MODE;
}

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

// *******************************************************
void myWiFi::APServer() {
    offline();
    WiFi.softAPConfig(local_ip, local_ip, subnet);
    WiFi.mode(WIFI_MODE_APSTA);
    delay(100);
    WiFi.persistent(false);
    WiFi.softAP(dssid, dpassword);
    WiFi.onEvent(WiFiClientConnected, SYSTEM_EVENT_AP_STACONNECTED);
    WiFi.onEvent(WiFiClientDisconnected, SYSTEM_EVENT_AP_STADISCONNECTED);
    m_mode = WIFI_APOINT_MODE;
    debug_println3("my_wifi", "APServer", String(WiFi.softAPIP().toString()).c_str());
}

void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info){
    debug_println3("my_wifi", "WiFiStationConnected", "Ok");
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info){
    debug_println3("my_wifi", "WiFiStationDisconnected", String("Trying to Reconnect").c_str());
    WiFi.disconnect();
    WiFi.reconnect();
}


// *******************************************************
bool myWiFi::tryConnectToWiFi(const char* ssid, const char* pass) {
    WiFi.begin(ssid, pass);
    debug_println3("my_wifi", "tryConnectToWiFi", String("ssid: " + String(ssid) + " pass: " + String(pass)).c_str());

    uint32_t try_to_connect_tmr = millis();

    while (millis() - try_to_connect_tmr < 30000) {
        if (WiFi.status() == WL_CONNECTED) {
            Serial.print("\n");
            WiFi.setAutoConnect(true);
            WiFi.persistent(true);
            WiFi.onEvent(WiFiStationConnected, SYSTEM_EVENT_STA_CONNECTED);
            WiFi.onEvent(WiFiStationDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);
            debug_println3("my_wifi", "tryConnectToWiFi", String(WiFi.localIP().toString()).c_str());
            debug_println3("my_wifi", "tryConnectToWiFi", String(WiFi.macAddress()).c_str());
            return true;
        }
        Serial.print(" . ");
        led.flash(1);
        delay(100);
    }

    if (WiFi.status() != WL_CONNECTED) {
        debug_println3("my_wifi", "tryConnectToWiFi", "false");
        return false;
    }
}

// *******************************************************
void myWiFi::offline() {
    WiFi.disconnect();
    delay(100);
    WiFi.mode(WIFI_OFF);
    delay(100);
    m_mode = WIFI_OFFLINE_MODE;
}

// *******************************************************
WifiMode myWiFi::getMode() {
    return m_mode;
}

/*
// *******************************************************
void myWiFi::checkNetwork() {
    if (millis() - check_network_tmr < 20000) {
        return;
    }

    check_network_tmr = millis();
    
    if (WiFi.status() == WL_CONNECTED) {
        return true;
    }
    
    change_preferably_network = true;
    
    WiFi.disconnect();
    
    if (tryConnectToWiFi(global_ssid, global_pass)) {
        net_offline_mode = false;
        change_preferably_network = false;
        return true;
    }
    else {
        Serial.println("testWifi. Error: connection timed out");
        WiFi.disconnect();
        return false;
    }
}
*/