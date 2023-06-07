#include "Arduino.h"
#include "config.h"
#include "my_ftp.h"
#include "ESP-FTP-Server-Lib.h"
#include "FTPFilesystem.h"
#include <SD.h>

FTPServer ftp;

TaskHandle_t FTPTaskHandle;

// *******************************************************
void myFTP::start() {
    ftp.addUser(FTP_USER, FTP_PASSWORD);
    ftp.addFilesystem("SD", &SD);
    ftp.begin();
}

// *******************************************************
void myFTP::handle() {
    ftp.handle();
}


/*
void FTPTask( void * param ) {
    debug_println3("my_ftp", "FTPTask", "start");
    myPIR *my_pir = static_cast<myPIR *>(param);
    while (true) {
        ftp.handle();
        vTaskDelay(10);
   }
}

// *******************************************************
void myFTP::run() {
    ftp.addUser(FTP_USER, FTP_PASSWORD);
    ftp.addFilesystem("SD", &SD);
    ftp.begin();

    xTaskCreate(FTPTask, "FTPTask", 4096, this, 1, &FTPTaskHandle);
    if (FTPTaskHandle == NULL) {
        debug_println3("my_ftp", "FTPTask", "failed");
    }
    else {
        debug_println3("my_ftp", "FTPTask", "task up and running");
    }
}

// *******************************************************
void myFTP::stop() {
    vTaskDelete(FTPTaskHandle);
    debug_println3("my_ftp", "FTPTask", "stoped");
}
*/
