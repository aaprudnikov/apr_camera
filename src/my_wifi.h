#ifndef MYWIFI_H
#define MYWIFI_H


enum RetryStatus {
    RETRY_NOT_NEEDED,
    RETRY_OK,
    RETRY_FAIL,
    RETRY_NO_NETWORKS
};


class myWiFi {
    private:
//        WifiMode m_mode;
        RetryStatus m_state;
    public:
        myWiFi();
        void APServer();
        bool tryConnectToWiFi(const char* ssid, const char* pass);
        void offline();

//        WifiMode getMode();
//        void setMode(WifiMode mode);
        RetryStatus establishingWiFiConnection();
        uint8_t scanNetCountInRange();
        uint8_t getNetCountInRange();
        String getSsidInRange(int id);
        RetryStatus getState();
};

#endif
