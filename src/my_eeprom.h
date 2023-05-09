#ifndef MY_EEPROM_H
#define MY_EEPROM_H

class myEeprom {
    private:
    public:
        myEeprom(){};
        bool eepromInit();
        uint16_t getCaptureCnt();
        void writeCaptureCnt(uint16_t counter);
        uint8_t getAutoCaptureFlag();
        void writeAutoCaptureFlag(uint8_t flag);
};

#endif
