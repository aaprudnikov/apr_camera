#include "EEPROM.h"
#include "my_eeprom.h"
#include "config.h"


bool myEeprom::eepromInit() {
    if (EEPROM.begin(3)) {
        debug_println3("my_eeprom", "eepromInit", "ok");
        return true;
    }
    else {
        debug_println3("my_eeprom", "eepromInit", "fail");
        return false;
    }
}

uint16_t myEeprom::getCaptureCnt() {
    uint16_t counter = EEPROM.readUShort(CAPTURE_CNT_ADDR);
    if (counter == 65535) counter = 0;
    debug_println3("my_eeprom", "getCaptureCnt", String(counter).c_str());
    return counter;
}


void myEeprom::writeCaptureCnt(uint16_t new_counter) {
    EEPROM.writeUShort(CAPTURE_CNT_ADDR, new_counter);
    EEPROM.commit();
    debug_println3("my_eeprom", "writeCaptureCnt", String(new_counter).c_str());
}

uint8_t myEeprom::getAutoCaptureFlag() {
    uint8_t flag = EEPROM.readByte(AUTO_CAPTURED_ADDR);
    debug_println3("my_eeprom", "getAutoCaptureFlag", String(flag).c_str());
    return flag;
}

void myEeprom::writeAutoCaptureFlag(uint8_t flag) {
    EEPROM.writeByte(AUTO_CAPTURED_ADDR, flag);
    EEPROM.commit();
    debug_println3("my_eeprom", "writeAutoCaptureFlag", String(flag).c_str());
}