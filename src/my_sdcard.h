#ifndef MYSDCARD_H
#define MYSDCARD_H

class mySDcard {
    private:
        bool m_sdcard_state = false;
    public:
        bool cardInit();
        bool checkFile(const char * path);
        bool createFile(const char * path);
        bool writeNetParamToFile(String ssid, String pass);
        bool checkKnownNetworks();
        bool writeFile(const char * path, const uint8_t *buf, size_t size);
        bool getInitState();
        bool savePhoto();
};

#endif
