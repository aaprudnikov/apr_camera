#include "Arduino.h"
#include "my_server.h"
#include "config.h"
#include <WebServer.h>
#include <my_wifi.h>
#include "config.h"
#include "my_sdcard.h"
#include "my_camera.h"
#include <Update.h>
#include <HTTPClient.h>

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

myServer::myServer(myWiFi *mWiFi) {
    m_wifi = mWiFi;
}

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
        } else {
            Update.printError(Serial);
            ota_start_flag = false;
        }
    }
}

void myServer::setupRoutingAPServer() {
    server.on("/", [=]() {

        char camera_state[7];
        char sdcard_state[7];
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
            if (m_wifi->tryConnectToWiFi(test_ssid.c_str(), test_pass.c_str())) {
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
                m_wifi->APServer();
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
    server.begin();
    setupRoutingAPServer();
    uint32_t initialise_tmr = millis();
    while (!initialise_flag) {
        server.handleClient();
        led.flash(1);
        if (!ota_start_flag && millis() - initialise_tmr > 60000 ) {
            debug_println3("my_server", "setupAPServer", "offlime mode");
            m_wifi->offline();
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
void myServer::setupRoutingHTTP() {

    server.on("/", HTTP_GET, [=]() {

        char camera_state[7];
        char sdcard_state[7];
        if (my_camera.getInitState()) strcpy(camera_state, "online");
        else strcpy(camera_state, "offline");
        if (sd.getInitState()) strcpy(sdcard_state, "online");
        else strcpy(sdcard_state, "offline");

        String html_content = "";
        html_content += "<form name='CAMERA'>";
        html_content += "<table width='30%' bgcolor='A09F9F' align='center'>";
        html_content += "<tr><td colspan=2><center><font size=4><b>The apr camera demo page</b></font></center><br></td><br><br></tr>";
        html_content += "<tr><td>camera date & time: </td> <td>" + String(my_ntp.getDataTimeLong()) + "</td></tr>";
        html_content += "<tr><td>main chip fw version: </td> <td>" + String(VERSION) + "</td></tr>";
        html_content += "</p><tr><td>camera: </td><td>" + String(camera_state) + "</td></tr>";
        html_content += "</p><tr><td>sd card: </td><td>" + String(sdcard_state) + "</td></tr>";
        html_content += "<tr><td>capture counter: </td> <td>" + String(my_eeprom.getCaptureCnt()) + "</td></tr>";
        html_content += "<tr><td>update manual:</td> <td><input type='submit' onclick=window.open('/ota') value='GO'></td></tr>";
        html_content += "<tr><td>capture: </td><td><input type='submit' onclick=window.open('/capture') value='Capture'></td></tr>";
        html_content += "<tr><td>use pir sensor: </td><td><input type='submit' onclick=window.open('/auto_captured_on') value='ON'><input type='submit' onclick=window.open('/auto_captured_off') value='OFF'></td></tr>";
        html_content += "<br></table></form>";
        html_content += "</html>";
        server.sendHeader("Connection", "close");
        server.send(200, "text/html", html_content);
    });

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
        String capture_path = String(CAPTURE_DIR) + "/" + String(web_capture_counter) + ".jpg";
        sd.writeFile(capture_path.c_str(), camera_fb, camera_fb_size);
        my_eeprom.writeCaptureCnt(web_capture_counter);
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
void myServer::setupMainServer() {
    setupRoutingHTTP();

    xTaskCreate(MainServerTask, "MainServerTask", 4096, this, 1, &MainServerTaskHandle);
	if (MainServerTaskHandle == NULL) {
        debug_println3("my_server", "setupMainServer", "failed");
	}
	else {
        debug_println3("my_server", "setupMainServer", "task up and running");
	}
}


void myServer::mhandleClient() {
    server.handleClient();
}