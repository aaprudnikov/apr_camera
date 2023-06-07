#include "Arduino.h"
#include "my_server.h"
#include "config.h"
#include <WebServer.h>
#include <my_wifi.h>
#include "my_sdcard.h"
#include "my_camera.h"
#include <Update.h>
#include <HTTPClient.h>
#include "my_pir.h"
#include "my_eeprom.h"
#include "my_ftp.h"

myFTP my_ftp;
//OV2640 my_camera;

WebServer server(80);

bool initialise_flag = false;
bool ota_start_flag = false;

TaskHandle_t MainServerTaskHandle;

// *******************************************************
const char* serverOTA = 
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form' align='center'>"
   "<input type='file' name='update'>"
        "<input type='submit' value='Update'>"
    "</form>"
 "<div id='prg' align='center'>progress: 0%</div>"
 "<script>"
  "$('form').submit(function(e){"
  "e.preventDefault();"
  "var form = $('#upload_form')[0];"
  "var data = new FormData(form);"
  " $.ajax({"
  "url: '/ota',"
  "type: 'POST',"
  "data: data,"
  "contentType: false,"
  "processData:false,"
  "xhr: function() {"
  "var xhr = new window.XMLHttpRequest();"
  "xhr.upload.addEventListener('progress', function(evt) {"
  "if (evt.lengthComputable) {"
  "var per = evt.loaded / evt.total;"
  "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
  "}"
  "}, false);"
  "return xhr;"
  "},"
  "success:function(d, s) {"
  "console.log('success!')"
 "},"
 "error: function (a, b, c) {"
 "}"
 "});"
 "});"
 "</script>";
/*
myServer::myServer(myWiFi *mWiFi) {
    m_wifi = mWiFi;
}
*/
void otaPost() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
        debug_println3("my_server", "OTA", String("Update: " + String(upload.filename.c_str())).c_str());

        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
            Update.printError(Serial);
            ota_start_flag = false;
        }

    } else if (upload.status == UPLOAD_FILE_WRITE) {
        ota_start_flag = true;
        led.on();
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
            Update.printError(Serial);
        }

    } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) { //true to set the size to the current progress

        debug_println3("my_server", "OTA", String("Update Successfull " + String(upload.totalSize)).c_str());
        delay(1000);
        led.flash(5);
        } else {
            Update.printError(Serial);
            ota_start_flag = false;
        }
    }
}

void myServer::setupRoutingAPServer() {
    server.on("/", [=]() {

        char camera_state[8];
        char sdcard_state[8];
        if (my_camera.getInitState()) strcpy(camera_state, "online");
        else strcpy(camera_state, "offline");
        if (sd.getInitState()) strcpy(sdcard_state, "online");
        else strcpy(sdcard_state, "offline");
        
        String html_content = "";
        html_content = "<!DOCTYPE HTML>\r\n<head><meta charset='utf-8'><title>Welcome to aprCamera configuration page</title></head>";
        html_content += "</p><tr><td>camera: </td><td>" + String(camera_state) + "</td></tr>";
        html_content += "</p><tr><td>sd card: </td><td>" + String(sdcard_state) + "</td></tr>";
        html_content += "<form ='get' action='config'>";
        html_content += "<label>SSID: </label><input type='text' name='ssid' length=16><br />";
        html_content += "<label>PASS: </label><input type='text' name='pass' length=36><br />";
        html_content += "<label>NTP: </label><input name='time_offset_value' length=2 value=\"3\"><br />";
        html_content += "<input type='submit'></form>";
        html_content += "</html>";
        server.send(200, "text/html", html_content);
    });

    server.on("/config", [=]() {
        String test_ssid = server.arg("ssid");
        String test_pass = server.arg("pass");
        String html_content = "";
        
        if (test_ssid.length() != 0 && test_pass.length() != 0) {
//            if (m_wifi->tryConnectToWiFi(test_ssid.c_str(), test_pass.c_str())) {
            if (my_wifi.tryConnectToWiFi(test_ssid.c_str(), test_pass.c_str())) {
                debug_println3("my_server", "setupRoutingAPServer", "OK");
                server.send(200, "text/html", "OK");
                sd.writeNetParamToFile(test_ssid, test_pass);
                initialise_flag = true;
                delay(1000);
                ESP.restart();
            }
            else {
                debug_println3("my_server", "setupRoutingAPServer", "fail");
                server.send(400, "text/html", "OK");
//                m_wifi->APServer();
                my_wifi.APServer();
            }
        }
    });

    server.on("/ota", HTTP_GET, []() {
        server.sendHeader("Connection", "close");
        server.send(200, "text/html", serverOTA);
    });

    server.on("/ota", HTTP_POST, []() {
        server.sendHeader("Connection", "close");
        server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
        ESP.restart();
    }, []() {
        otaPost();
    });
}

// *******************************************************
void myServer::setupAPServer() {
    my_wifi.APServer();
    server.begin();
    setupRoutingAPServer();
    uint32_t initialise_tmr = millis();
    while (!initialise_flag) {
        server.handleClient();
        led.flash(1);
        if (!ota_start_flag && millis() - initialise_tmr > 60000 ) {
            debug_println3("my_server", "setupAPServer", "offlime mode");
            my_wifi.offline();
            break;
        }
        delay(500);
    }
    
    server.stop();
}

// *******************************************************
void handleNotFound() {
	String message = "oops..";
	message += "\n";
	server.send(200, "text/plain", message);
}

// *******************************************************
void setIntervalCapture(int) {
}


// *******************************************************
/*
void handleCaptureRoot() {
  String res = sd.printCaptureDirectory(CAPTURE_DIR, 0);
  server.send(200, "text/html", res);
}
*/
// *******************************************************
void myServer::setupRoutingHTTP() {

    server.on("/", HTTP_GET, [=]() {

        char camera_state[8];
        char sdcard_state[8];
        if (my_camera.getInitState()) strcpy(camera_state, "online");
        else strcpy(camera_state, "offline");
        if (sd.getInitState()) strcpy(sdcard_state, "online");
        else strcpy(sdcard_state, "offline");

        String html_content = "";
        html_content += "<form name='CAMERA'>";
        html_content += "<table width='30%' bgcolor='A09F9F' align='center'>";
        html_content += "<tr><td colspan=2><center><font size=4><b>The apr camera demo page</b></font></center><br></td><br><br></tr>";
//        html_content += "<tr><td>camera date & time: </td> <td>" + String(my_ntp.getDataTimeLong()) + "</td></tr>";
        html_content += "<tr><td>uptime (sec): </td> <td>" + String(millis()/1000) + "</td></tr>";
        html_content += "<tr><td>main chip fw version: </td> <td>" + String(VERSION) + "</td></tr>";
        html_content += "<tr><td>free heap: </td> <td>" + String(ESP.getFreeHeap()) + " byte</td></tr>";
        html_content += "<tr><td>free storage: </td> <td>" + String(sd.getFreePercentSpace()) + " %</td></tr>";
        html_content += "</p><tr><td>number of launches: </td><td>" + String(my_eeprom.readUShort(SWITCHING_ON_CNT)) + "</td></tr>";
        html_content += "</p><tr><td>number of successful launches: </td><td>" + String(my_eeprom.readUShort(SWITCHING_ON_SUCC_CNT)) + "</td></tr>";
//        html_content += "</p><tr><td>camera: </td><td>" + String(camera_state) + "</td></tr>";
//        html_content += "</p><tr><td>sd card: </td><td>" + String(sdcard_state) + "</td></tr>";
        html_content += "<tr><td>capture counter: </td> <td>" + String(my_eeprom.getCaptureCnt()) + "</td></tr>";
        html_content += "<tr><td>update manual:</td> <td><input type='submit' onclick=window.open('/ota') value='GO'></td></tr>";
        html_content += "<tr><td>capture: </td><td><input type='submit' onclick=window.open('/capture') value='Capture'></td></tr>";
//        html_content += "<tr><td>show captured files: </td><td><input type='submit' onclick=window.open('/capture_dir') value='Go'></td></tr>";
        html_content += "<tr><td>use pir sensor: </td><td><input type='submit' onclick=window.open('/auto_captured_on') value='ON'><input type='submit' onclick=window.open('/auto_captured_off') value='OFF'></td></tr>";
//        html_content += "<tr><td>ftp server: </td><td><input type='submit' onclick=window.open('/ftp_on') value='ON'><input type='submit' onclick=window.open('/ftp_off') value='OFF'></td></tr>";
/*
        html_content += "<br></table></form>";
        html_content += "<form method='get' action='interval'>";
        html_content += "<table width='30%' bgcolor='A09F9F' align='center'>";
        html_content += "<tr><td colspan=2><center><font size=4><b>Interval capture (sec):</b></font></center></td></tr>";
        html_content += "<tr><td><input name='interval'></td><td><input type='submit'></form></td></tr>";
        html_content += "</table></form>";

        html_content += "<form method='get' action='show_capture'>";
        html_content += "<table width='30%' bgcolor='A09F9F' align='center'>";
        html_content += "<tr><td colspan=2><center><font size=4><b>Show capture by number:</b></font></center></td></tr>";
        html_content += "<tr><td><input name='show_capture'></td><td><input type='submit'></form></td></tr>";
        html_content += "</table></form>";
*/
        html_content += "</html>";
        server.sendHeader("Connection", "close");
        server.send(200, "text/html", html_content);
    });
/*
    server.on("/show_capture", [=]() {
        String show_capture = String(sd.getWorkingDirectory()) + "/" + server.arg("show_capture") + ".jpg";
        debug_println3("setupRoutingHTTP", "show_capture", show_capture.c_str());

        if (show_capture.length() != 0) {
            if (sd.checkFile(show_capture.c_str())) {
                debug_println3("setupRoutingHTTP", "show_capture", "file exists");

                WiFiClient thisClient = server.client();

                if (!thisClient.connected()) {
                    return;
                }

                String response = "HTTP/1.1 200 OK\r\n";
                response += "Content-disposition: inline; filename=capture.jpg\r\n";
                response += "Content-type: image/jpeg\r\n\r\n";
                server.sendContent(response);

                uint8_t * file_fb = sd.getFileData(show_capture.c_str());
                size_t file_fb_size = sd.getFileSize(show_capture.c_str());
                thisClient.write((char *)file_fb, file_fb_size);
                server.sendContent("\r\n");
                thisClient.flush();

//                server.sendHeader("Connection", "close");
//                server.send(200, "image/png", "OK");
            }
            else {
                debug_println3("setupRoutingHTTP", "show_capture", "file does not exists");
                server.sendHeader("Connection", "close");
                server.send(400, "text/html", "Bad reauest");
            }
        }
        else {
            server.sendHeader("Connection", "close");
            server.send(400, "text/html", "Bad reauest");
        }
    });
*/
/*
    server.on("/capture/500", HTTP_GET, []() {
        my_eeprom.writeAutoCaptureFlag(0);
        server.sendHeader("Connection", "close");
        server.send();
        server.send(200, "image/png", "OK");
    });
*/
/*
    server.on("/interval", [=]() {
        String interval_str = server.arg("interval");
        debug_println3("setupRoutingHTTP", "interval", interval_str);
        if (interval_str.length() != 0) {
            setIntervalCapture(strtol(interval_str.c_str(), NULL, 0));
            server.sendHeader("Connection", "close");
            server.send(200, "text/html", "OK");
        }
        else {
            server.sendHeader("Connection", "close");
            server.send(400, "text/html", "Bad reauest");
        }
    });
*/
    server.on("/auto_captured_on", HTTP_GET, []() {
        my_eeprom.writeAutoCaptureFlag(1);
        server.sendHeader("Connection", "close");
        server.send(200, "text/html", "OK");
    });

    server.on("/auto_captured_off", HTTP_GET, []() {
        my_eeprom.writeAutoCaptureFlag(0);
        server.sendHeader("Connection", "close");
        server.send(200, "text/html", "OK");
    });
/*    
    server.on("/ftp_on", HTTP_GET, []() {
        my_ftp.run();
        server.sendHeader("Connection", "close");
        server.send(200, "text/html", "OK");
    });

    server.on("/ftp_off", HTTP_GET, []() {
        my_ftp.stop();
        server.sendHeader("Connection", "close");
        server.send(200, "text/html", "OK");
    });
*/
    server.on("/capture", HTTP_GET, [=]() {
        
        led.flash(1);

        WiFiClient thisClient = server.client();

        if (!my_camera.getInitState() || !sd.getInitState()) {
            String response = "HTTP/1.1 400 check camera module or sd card\r\n";
            server.sendContent(response);
            return;
        }

        if (!thisClient.connected()) {
            return;
        }

        String response = "HTTP/1.1 200 OK\r\n";
        response += "Content-disposition: inline; filename=capture.jpg\r\n";
        response += "Content-type: image/jpeg\r\n\r\n";
        server.sendContent(response);

        my_camera.run();
        uint8_t * camera_fb = my_camera.getfb();
        size_t camera_fb_size = my_camera.getSize();
        thisClient.write((char *)camera_fb, camera_fb_size);
        server.sendContent("\r\n");
        thisClient.flush();

        uint16_t web_capture_counter = my_eeprom.getCaptureCnt() + 1;
//        String capture_path = String(WEB_CAPTURE_DIR) + "/" + my_ntp.getDataTimeSplit() + "_" + String(web_capture_counter) + ".jpg";
        String capture_path = String(sd.getWorkingDirectory()) + "/" + String(web_capture_counter) + ".jpg";
        sd.writeFile(capture_path.c_str(), camera_fb, camera_fb_size);
        my_eeprom.writeCaptureCnt(web_capture_counter);
    });

//    server.on("/capture_dir", handleCaptureRoot);

    server.on("/ota", HTTP_GET, []() {
        server.sendHeader("Connection", "close");
        server.send(200, "text/html", serverOTA);
    });
    
    server.on("/ota", HTTP_POST, []() {
        server.sendHeader("Connection", "close");
        server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
        ESP.restart();
    }, []() {
        otaPost();
    });
    server.onNotFound(handleNotFound);
    server.begin();
    debug_println3("my_server", "setupRoutingHTTP", "start");
}

// *******************************************************
void MainServerTask( void * param ) {
    debug_println3("my_server", "MainServerTask", "start");
    while (true) {
        server.handleClient();
        vTaskDelay(100);
    }
}

// *******************************************************
void myServer::startMainServer() {
    setupRoutingHTTP();

    xTaskCreate(MainServerTask, "MainServerTask", 4096, this, 1, &MainServerTaskHandle);
    if (MainServerTaskHandle == NULL) {
        debug_println3("my_server", "startMainServer", "failed");
    }
    else {
        debug_println3("my_server", "startMainServer", "task up and running");
    }
}

void myServer::setupRouting() {
    setupRoutingHTTP();
}

void myServer::mhandleClient() {
    server.handleClient();
}

// *******************************************************
void myServer::stopMainServer() {
    debug_println3("my_server", "MainServerTask", "stop");
    vTaskDelete(MainServerTaskHandle);
}