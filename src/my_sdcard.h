#ifndef MYSDCARD_H
#define MYSDCARD_H

#include "FS.h"

class mySDcard {
    private:
        bool m_sdcard_state = false;
    public:
        bool cardInit();
        void createDir(fs::FS &fs, const char * path);
        bool checkFile(const char * path);
        bool writeNetParamToFile(String ssid, String pass);
        bool checkKnownNetworks();
        bool writeFile(const char * path, const uint8_t *buf, size_t size);
        uint8_t *getFileData(const char * path);
        size_t getFileSize(const char * path);
        bool getInitState();
        bool savePhoto();
        String getWorkingDirectory();
        int getFreePercentSpace();
//        String printCaptureDirectory(const char * path, int numTabs);

};

#endif
