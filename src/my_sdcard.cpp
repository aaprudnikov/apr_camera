#include "Arduino.h"
#include "SD.h"
#include "FS.h"
#include "my_sdcard.h"
#include "config.h"
#include <ArduinoJson.h>
#include "my_wifi.h"
#include "my_camera.h"

enum netParam {
  SSID,
  PASS
};

enum cryptoMode {
  ENCODE,
  DECODE
};

// *******************************************************
void mySDcard::createDir(fs::FS &fs, const char * path) {
  if(!SD.open(path)) {
    if(fs.mkdir(path)) {
      debug_println3("my_sdcard", String("createDir: " + String(path)).c_str(), "ok")
    }
    else {
      debug_println3("my_sdcard", String("createDir: " + String(path)).c_str(), "fail")
    }
  }
}

// *******************************************************
bool mySDcard::cardInit() {
  SPI.begin(14, 2, 15, 13);
  if(!SD.begin(13)){
      Serial.println("SD Card Mount Failed");
      debug_println2("my_sdcard", "init error");
      m_sdcard_state = false;
      return false;
  }
  
  uint8_t cardType = SD.cardType();
  if(cardType == CARD_NONE){
      debug_println2("my_sdcard", "no card attached");
      m_sdcard_state = false;
      return false;
  }
/*
  if(!SD.open(WEB_CAPTURE_DIR)){
    createDir(SD, WEB_CAPTURE_DIR);
  }
  if(!SD.open(PIR_CAPTURE_DIR)){
    createDir(SD, PIR_CAPTURE_DIR);
  }
  if(!SD.open(CAPTURE_DIR)){
    createDir(SD, CAPTURE_DIR);
  }
*/

  debug_println2("my_sdcard", "init ok");
  m_sdcard_state = true;
  return true;
}

// *******************************************************
bool mySDcard::checkFile(const char * path) {

  if (SD.exists(path)) {
      debug_println3("my_sdcard", "checkFile: file exists", String(path).c_str());
      return true;
  }
  else {
      debug_println3("my_sdcard", "checkFile: file does not exists", String(path).c_str());
      return false;
  }
}


// *******************************************************
bool mySDcard::getInitState() {
  return m_sdcard_state;
}


// *******************************************************
String cryptoString(String input_string, uint8_t mode) {
  char tmp_input_string[96];
  String output_string = "";
  uint8_t input_string_length = input_string.length();
  strcpy(tmp_input_string, input_string.c_str());
  switch(mode) {
    case ENCODE:
      for (int i = 0; i < input_string_length; i++) {
          if (int(tmp_input_string[i]) + int('1') > 255) {
            output_string += char(int('1') - (255 - int(tmp_input_string[i])));
          } else {
            output_string += char(int(tmp_input_string[i]) + int('1'));
          }
      }
      break;

    case DECODE:
      for (int i = 0; i < input_string_length; i++) {
          if (int(tmp_input_string[i]) - int('1') < 0) {
            output_string += char(255 - (int('1') - int(tmp_input_string[i])));
          } else {
            output_string += char(int(tmp_input_string[i]) - int('1'));
          }
      }
      break;
  }
  return output_string;
}

// *******************************************************
bool mySDcard::writeNetParamToFile(String ssid, String pass) {
  StaticJsonDocument<128> jsonDocumentFile;
  char bufferFile[128];
  
  File file_w;

  if(!SD.exists(NETWORK_FILE)) {
    file_w = SD.open(NETWORK_FILE, FILE_WRITE);
    debug_println3("my_sdcard", "writeNetParamToFile", "file created");
  }  
  else {
    file_w = SD.open(NETWORK_FILE, FILE_APPEND);
    debug_println3("my_sdcard", "writeNetParamToFile", "file appended");
  }
 
  if (file_w) {
    jsonDocumentFile.clear();
    jsonDocumentFile["ssid"] = ssid;
    jsonDocumentFile["pass"] = pass;
    serializeJson(jsonDocumentFile, bufferFile);
    file_w.println(cryptoString(bufferFile, ENCODE));
    file_w.close();
    debug_println3("my_sdcard", "writeNetParamToFile", "network successful saved in file");
    jsonDocumentFile.clear();
    return true;
  }
  else {
    debug_println3("my_sdcard", "writeNetParamToFile", "error");
    jsonDocumentFile.clear();
    return false;
  }
}

// *******************************************************
String getNetParamFromFile(String string, uint8_t param) {
  StaticJsonDocument<128> jsonDocumentFile;
  deserializeJson(jsonDocumentFile, string);

  switch(param) {
    case SSID:
      return jsonDocumentFile["ssid"];
      break;
    case PASS:
      return jsonDocumentFile["pass"];
      break;
  }
  return "error";
  jsonDocumentFile.clear();
}

// *******************************************************
bool mySDcard::checkKnownNetworks() {
  myWiFi m_wifi;
  uint8_t row = 0; 
  uint8_t col = 0;       
  File file_r = SD.open(NETWORK_FILE);
  String encoded_network_from_file;

  while(file_r.available()) {
      char symbol = char(file_r.read());
      if (symbol == '\n') {
        String network_from_file_tmp = cryptoString(encoded_network_from_file, DECODE);
        encoded_network_from_file.clear();
        debug_println3("my_sdcard", "checkKnownNetworks", network_from_file_tmp.c_str());
        String ssid_tmp = getNetParamFromFile(network_from_file_tmp, SSID).c_str();
        uint8_t networks_in_range = m_wifi.getNetCountInRange();
        for (int i = 0; i < networks_in_range; i++) {
          debug_println3("my_sdcard", "compare", String(m_wifi.getSsidInRange(i) + " to: " + ssid_tmp).c_str());
          if (m_wifi.getSsidInRange(i) == ssid_tmp) {
            if (m_wifi.tryConnectToWiFi(ssid_tmp.c_str(), getNetParamFromFile(network_from_file_tmp, PASS).c_str())) {
              debug_println3("my_sdcard", "checkKnownNetworks", "found");
              file_r.close();
              return true;
            }
          }
        }
        row++; 
        col=0;
      }
      else {
          encoded_network_from_file += symbol;
          col++;
      }
    }
    debug_println3("my_sdcard", "checkKnownNetworks", "No matches");
    file_r.close();
    return false;
}

// *******************************************************
bool mySDcard::writeFile(const char * path, const uint8_t *buf, size_t size) {
  File file_w;
  
  String file_path = path;

  if (SD.exists(path)) {
    file_path += "_" + String(my_eeprom.getCaptureCnt());
  }

  file_w = SD.open(file_path.c_str(), FILE_WRITE);
 
  if (!file_w) {
    debug_println3("my_sdcard", "writeFile", String("file error: " + file_path).c_str());
    return false;
  }

  debug_println3("my_sdcard", "writeFile", "atempt to write");
  file_w.write(buf, size);
  file_w.close();
  debug_println3("my_sdcard", "writeFile", String("write done: " + file_path).c_str());
  return true;
}

// *******************************************************
bool mySDcard::savePhoto() {
  if (!sd.getInitState()) {
    debug_println3("my_sdcard", "savePhoto", "error: no sd card");
    return false;
  }

  if (!my_camera.getInitState()) {
    debug_println3("my_sdcard", "savePhoto", "error: no camera found");
    return false;
  }

//  OV2640 my_camera;

  File file_w;

  uint16_t capture_counter = my_eeprom.getCaptureCnt() + 1;
  String file_path = String(getWorkingDirectory()) + "/" + String(capture_counter) + ".jpg";

  file_w = SD.open(file_path.c_str(), FILE_WRITE);

  if (!file_w) {
    debug_println3("my_sdcard", "savePhoto", String("file error: " + file_path).c_str());
    return false;
  }

  my_camera.run();
  file_w.write(my_camera.getfb(), my_camera.getSize());
  file_w.close();
  debug_println3("my_sdcard", "savePhoto", String("write done: " + file_path).c_str());

  my_eeprom.writeCaptureCnt(capture_counter);
  return true;
}

// *******************************************************
/*
String mySDcard::printCaptureDirectory(const char * path, int numTabs) {
  String response = "";
  File path_dir = SD.open(path);

  path_dir.rewindDirectory();
  
  while(true) {
    File entry =  path_dir.openNextFile();
    if (!entry) {
      break;
    }
    for (uint8_t i=0; i<numTabs; i++) {
      Serial.print('\t');
    }
    if (entry.isDirectory()) {
      printCapturedDirectory(String(entry).c_str(), numTabs+1);
    } else {
      response += String("<a href='") + String(entry.name()) + String("'>") + String(entry.name()) + String("</a>") + String("</br>");
      debug_println3("my_sdcard", "printCapturedDirectory", String(entry.name()).c_str());
    }
    entry.close();
  }
  return String("List files:</br>") + response;
}
*/

// *******************************************************
size_t mySDcard::getFileSize(const char * path) {
  File file = SD.open(path);

  if (!file) {
    return 0;
  }
  debug_println3("my_sdcard", "getFileSize", String(file.size()).c_str());
  return file.size();
}

// *******************************************************
uint8_t * mySDcard::getFileData(const char * path) {

  File file = SD.open(path);

  if (!file) {
    return 0;
  }

  size_t size = getFileSize(path);
  uint8_t *buff = (uint8_t *) malloc(size);
  file.read(buff, size);

  return buff;
}

// *******************************************************
String mySDcard::getWorkingDirectory() {
  String work_directory = String(CAPTURE_DIR_PREFIX) + "_";                           // "/capture_"
  int postfix_cur = int(my_eeprom.readUShort(CAPTURE_CNT_ADDR) / 500);        // текущий постфикс к рабочей директории
  if (postfix_cur != my_eeprom.read(CAPTURE_DIR_POSTFIX_CNT)) {               // если текущий не равен сохраненному, то
    postfix_cur ++;                                                           // инкремент 
    if (postfix_cur > 255) {                                                  // проверка на переполнение
      my_eeprom.write(CAPTURE_DIR_POSTFIX_CNT, 0);                            // если да, то 0
      postfix_cur = 0;
      createDir(SD, String(work_directory + String(0)).c_str());              // на всякий случай проверяем есть ли такая
    }
    else {
      my_eeprom.write(CAPTURE_DIR_POSTFIX_CNT, postfix_cur);                  // если нет, то записываем новое значение
      createDir(SD, String(work_directory + String(postfix_cur)).c_str());    // и создаем директорию
    }
  }
  work_directory += String(postfix_cur);                                       //
  debug_println3("my_sdcard", "getWorkingDirectory", work_directory.c_str());

  return work_directory;
}

int mySDcard::getFreePercentSpace() {
  if (!m_sdcard_state) return 0;
  return int((1 - SD.usedBytes() / SD.totalBytes()) * 100);
}