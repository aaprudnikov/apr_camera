#include "EEPROM.h"
#include "my_eeprom.h"
#include "config.h"
#include "my_mqtt.h"

bool myEeprom::eepromInit() {
    if (EEPROM.begin(NVS_VOLUME)) {
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
//    debug_println3("my_eeprom", "getCaptureCnt", String(counter).c_str());
    return counter;
}

void myEeprom::writeCaptureCnt(uint16_t new_counter) {
    EEPROM.writeUShort(CAPTURE_CNT_ADDR, new_counter);
    EEPROM.commit();
    debug_println3("my_eeprom", "writeCaptureCnt", String(new_counter).c_str());
    my_mqtt.sendData(MY_CAPTURE_CNT, new_counter);
}

uint8_t myEeprom::getAutoCaptureFlag() {
    uint8_t flag = EEPROM.readByte(AUTO_CAPTURED_ADDR);
//    debug_println3("my_eeprom", "getAutoCaptureFlag", String(flag).c_str());
    return flag;
}

void myEeprom::writeAutoCaptureFlag(uint8_t flag) {
    EEPROM.writeByte(AUTO_CAPTURED_ADDR, flag);
    EEPROM.commit();
    debug_println3("my_eeprom", "writeAutoCaptureFlag", String(flag).c_str());
}

uint16_t myEeprom::readUShort(int address) {
    return EEPROM.readUShort(address);
}

uint8_t myEeprom::read(int address) {
    return EEPROM.read(address);
}

void myEeprom::writeUShort(int address, uint16_t value) {
    EEPROM.writeUShort(address, value);
    EEPROM.commit();
    debug_println3("my_eeprom", "writeUShort", String("address: " + String(address) + " value: " + String(value)).c_str());
}

void myEeprom::write(int address, uint8_t value) {
    EEPROM.write(address, value);
    EEPROM.commit();
    debug_println3("my_eeprom", "write", String("address: " + String(address) + " value: " + String(value)).c_str());
}