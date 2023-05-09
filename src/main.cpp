#include "Arduino.h"
#include "config.h"
#include "my_sdcard.h"
#include "my_led.h"
#include "my_wifi.h"
#include "my_server.h"
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "my_camera.h"
#include "my_ntp.h"
#include "my_eeprom.h"
#include "my_pir.h"

myLed led;
myWiFi *wifi = new myWiFi();
mySDcard sd;
myServer *mserver = new myServer(wifi);
OV2640 my_camera;
myNTP my_ntp;
myEeprom my_eeprom;
myPIR my_pir;

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  Serial.begin(115200);
  
  debug_println2("main", "start");

  uint8_t camera_init_cnt = 0;
  uint8_t sdcard_init_cnt = 0;
  
  while (my_camera.init(cam_config) != ESP_OK) {
    led.flash(1);
    camera_init_cnt++;
    if (camera_init_cnt > 3) {
      break;
    }
    delay(5000);
  }
  
  while (!sd.cardInit()) {
    led.flash(2);
    sdcard_init_cnt++;
    if (sdcard_init_cnt > 3) {
      break;
    }
    delay(5000);
  }

  while (!my_eeprom.eepromInit()) {
    delay(5000);
  }

  wifi->clientMode();

  if (!sd.checkFile(NETWORK_FILE) || 
      !sd.checkKnownNetworks() || 
      !my_camera.getInitState()||
      !sd.getInitState()) {
    wifi->APServer();
    mserver->setupAPServer();
  }
  else if (!my_camera.getInitState()|| !sd.getInitState()) {
    debug_println3("main", "deep sleep", "check camera module or sd card");
    delay(500);
    esp_deep_sleep_start();
  }

  if (wifi->getMode() != WIFI_OFFLINE_MODE) {
    my_ntp.getTime();
    my_pir.run(PIR_OBSERVER);
    mserver->setupMainServer();
  }
  else {
    my_pir.run(PIR_GUARD);
  }
}

void loop() {
  delay(100);
}
