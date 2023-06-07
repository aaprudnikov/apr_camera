#include "Arduino.h"
#include "config.h"
#include "my_sdcard.h"
#include "my_led.h"
#include "my_wifi.h"
#include "my_server.h"
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "my_camera.h"
//#include "my_ntp.h"
#include "my_eeprom.h"
#include "my_pir.h"
#include "my_mqtt.h"

myLed led;
OV2640 my_camera;
//myWiFi *wifi = new myWiFi();
myWiFi my_wifi;
mySDcard sd;
//myServer *mserver = new myServer(wifi);
myServer my_server;
//myNTP my_ntp;
myEeprom my_eeprom;
myPIR my_pir;
myMQTT my_mqtt;

bool wifi_state_online_flag = false;
bool wifi_ap_at_start_flag = true;


// *******************************************************
void checkNetwork() {
  static uint16_t retry_cnt;
  static uint32_t retry_tmr;

  if (retry_cnt > 0) {
    if (wifi_state_online_flag) {
      wifi_state_online_flag = false;
      my_wifi.offline();
    }

    if (millis() - retry_tmr < 60000) {
      return;
    }
  }

  switch (my_wifi.establishingWiFiConnection()) {

    case RETRY_OK:
//      my_ntp.getTime();
      my_pir.setState(PIR_OBSERVER);
      my_server.setupRouting();
      my_ftp.start();
      retry_cnt = 0;
      wifi_state_online_flag = true;
      break;

    case RETRY_FAIL:
      my_pir.setState(PIR_GUARD);
      if (wifi_ap_at_start_flag) {
        my_server.setupAPServer();
        wifi_ap_at_start_flag = false;
      }
      retry_cnt++;
      wifi_state_online_flag = false;
      break;

    case RETRY_NOT_NEEDED:
      static uint32_t mqtt_publish_tmr;
      if (millis() - mqtt_publish_tmr > 60000) {
        mqtt_publish_tmr = millis();
        my_mqtt.sendData(MY_UPTIME, millis()/1000);
      }
      my_mqtt.retringSendData();
      my_server.mhandleClient();
      my_ftp.handle();
      retry_cnt = 0;
      wifi_state_online_flag = true;
      break;

    case RETRY_NO_NETWORKS:
        my_pir.setState(PIR_GUARD);
        retry_cnt++;
        wifi_state_online_flag = false;
      break;
  }

  retry_tmr = millis();
}

// *******************************************************
void espDeepSleep() {
  debug_println3("main", "deep sleep", "check camera module or sd card");   
  led.flash(5);
  delay(500);
  ESP.restart();
//      esp_deep_sleep_start();
}

// *******************************************************
void debugLogging() {
  static uint32_t log_tmr;
  if (millis() - log_tmr < 1000) return;

  log_tmr = millis();
//  debug_println1(String(String(my_ntp.getDataTimeLong()) + " " + String(ESP.getFreeHeap()) + " " + String(my_eeprom.getCaptureCnt())).c_str());   
  debug_println1((String(ESP.getFreeHeap()) + " " + String(my_eeprom.getCaptureCnt())).c_str());   
}

// *******************************************************
void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  Serial.begin(115200);
  led.on();
  
  debug_println2("main", "start");

  uint8_t camera_init_cnt = 0;
  uint8_t sdcard_init_cnt = 0;
  
  while (!my_eeprom.eepromInit()) {                                             // инициализация nvs
    led.flash(3);
    delay(5000);
  }
  my_eeprom.writeUShort(SWITCHING_ON_CNT, my_eeprom.readUShort(SWITCHING_ON_CNT) + 1);

  while (my_camera.init(camera_config) != ESP_OK && camera_init_cnt < 3) {                                         // инициализация камеры
    led.flash(1);
    camera_init_cnt++;
    delay(5000);
  }

  while (!sd.cardInit() && sdcard_init_cnt < 3) {                                                      // инициализация карты памяти
    led.flash(2);
    sdcard_init_cnt++;
    delay(5000);
  }

  if (!my_camera.getInitState() || !sd.getInitState()) {                      // если проблема с камерой или картой, то рестарт
      my_wifi.APServer();
      my_server.setupAPServer();
  }
  else {
    my_eeprom.writeUShort(SWITCHING_ON_SUCC_CNT, my_eeprom.readUShort(SWITCHING_ON_SUCC_CNT) + 1);
  }
}

// *******************************************************
void loop() {
  checkNetwork();
  debugLogging();
  my_pir.handle();
}
