#ifndef MYWIFI_H
#define MYWIFI_H

enum WifiMode {
    WIFI_APOINT_MODE,
    WIFI_CLIENT_MODE,
    WIFI_OFFLINE_MODE
};

class myWiFi {
    private:
        WifiMode m_mode; 
    public:
        myWiFi();
        void APServer();
        void clientMode();
        bool tryConnectToWiFi(const char* ssid, const char* pass);
        void offline();
        WifiMode getMode();
        void checkNetwork();
};

#endif
