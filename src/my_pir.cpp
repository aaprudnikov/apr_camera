#include "Arduino.h"
#include "my_pir.h"
#include "config.h"

TaskHandle_t PIRTaskHandle;

// *******************************************************
void PIRTask( void * param ) {
    debug_println3("my_pir", "PIRTask", "start");
    bool detection_flag = false;
    myPIR *my_pir = static_cast<myPIR *>(param);
    uint32_t last_detection_tmr;
    while (true) {
        if (digitalRead(PIR_PIN) && millis() - last_detection_tmr > 5000) {
            if (!detection_flag) {
                last_detection_tmr = millis();
                detection_flag = true;
                switch (my_pir->getState()) {
                    case PIR_GUARD:
                        debug_println3("my_pir", "PIRTask", "PIR_GUARD");
                        sd.savePhoto();
                        break;
                    case PIR_OBSERVER:
                        debug_println3("my_pir", "PIRTask", "PIR_OBSERVER");
                        if (my_eeprom.getAutoCaptureFlag() == 1) {
                            sd.savePhoto();
                        }
                        break;
                }
            }
            if (millis() - last_detection_tmr > 5000) {
                detection_flag = false;
            }
        }
        else {
            detection_flag = false;
        }
        vTaskDelay(10);
    }
}

// *******************************************************
void myPIR::handle() {
    bool detection_flag;
    static uint32_t last_detection_tmr;

    if (digitalRead(PIR_PIN)) {
        if (millis() - last_detection_tmr > 5000) {
            last_detection_tmr = millis();
            switch (getState()) {
                case PIR_GUARD:
                    debug_println3("my_pir", "PIRTask", "PIR_GUARD");
                    sd.savePhoto();
                    break;
                case PIR_OBSERVER:
                    debug_println3("my_pir", "PIRTask", "PIR_OBSERVER");
                    if (my_eeprom.getAutoCaptureFlag() == 1) {
                        sd.savePhoto();
                    }
                    break;
            }
        }
    }
}

/*
// *******************************************************
void myPIR::run(PirMode mode) {
    pinMode(PIR_PIN, INPUT);
    m_mode = mode;

    xTaskCreate(PIRTask, "PIRTask", 4096, this, 1, &PIRTaskHandle);
    if (PIRTaskHandle == NULL) {
        debug_println3("my_pir", "PIRTask", "failed");
    }
    else {
        debug_println3("my_pir", "PIRTask", "task up and running");
    }
}

// *******************************************************
void myPIR::stop() {
    debug_println3("my_pir", "PIRTask", "stop");
    vTaskDelete(PIRTaskHandle);
}
*/
// *******************************************************
PirMode myPIR::getState() {
    return m_mode;
}

// *******************************************************
void myPIR::setState(PirMode mode) {
    m_mode = mode;
 }