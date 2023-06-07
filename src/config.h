#ifndef MYCONFIG_H
#define MYCONFIG_H

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#include "my_wifi.h"
extern myWiFi my_wifi;

#include "my_server.h"
extern myServer my_server;

#include "my_camera.h"
extern OV2640 my_camera;

//#include "my_ntp.h"
//extern myNTP my_ntp;

#include "my_sdcard.h"
extern mySDcard sd;

#include "my_led.h"
extern myLed led;

#define DEBUG 1
#define debug_println1(text1) { if (DEBUG) printf("[ %i ][ %s ]\n", int(millis()), text1); };
#define debug_println2(text1, text2) { if (DEBUG) printf("[ %i ][ %s ][ %s ]\n", int(millis()), text1, text2); };
#define debug_println3(text1, text2, text3) { if (DEBUG) printf("[ %i ][ %s ][ %s ][ %s ]\n", int(millis()), text1, text2, text3); };

#define LED 33

#define NETWORK_FILE "/networks.cfg"
//#define WEB_CAPTURE_DIR "/capture_web"
//#define PIR_CAPTURE_DIR "/capture_pir"
#define CAPTURE_DIR_PREFIX "/capture"

#define VERSION "0.2.1"

#define NVS_VOLUME 8
#define CAPTURE_CNT_ADDR 0      // 2 bytes
#define AUTO_CAPTURED_ADDR 2    // 1 byte
#define SWITCHING_ON_CNT 3          // 2 bytes
#define SWITCHING_ON_SUCC_CNT 5     // 2 bytes
#define CAPTURE_DIR_POSTFIX_CNT 1     // 1 bytes

#include "my_eeprom.h"
extern myEeprom my_eeprom;

#define PIR_PIN 16
#define BUTTON_PIN 100

#include "my_pir.h"
extern myPIR my_pir;

#include "my_ftp.h"
extern myFTP my_ftp;
#define FTP_USER "ftp"
#define FTP_PASSWORD "ftp"

#include "my_mqtt.h"
extern myMQTT my_mqtt;

#endif
