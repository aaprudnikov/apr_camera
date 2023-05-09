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

#include "my_camera.h"
extern OV2640 my_camera;

#include "my_ntp.h"
extern myNTP my_ntp;

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
#define WEB_CAPTURE_DIR "/capture_web"
#define PIR_CAPTURE_DIR "/capture_pir"
#define CAPTURE_DIR "/capture"

#define VERSION "0.0.1"

#define CAPTURE_CNT_ADDR 0
#define AUTO_CAPTURED_ADDR 2

#include "my_eeprom.h"
extern myEeprom my_eeprom;

#define PIR_PIN 16
#define BUTTON_PIN 100

#endif
