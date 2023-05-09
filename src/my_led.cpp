#include "Arduino.h"
#include "config.h"
#include "my_led.h"

myLed::myLed() {
    pinMode(LED, OUTPUT);
    digitalWrite(LED, HIGH);
}

void myLed::flash(int count) {
    for (uint8_t n = 0 ; n < count ; n ++) {
        digitalWrite(33,LOW);
        delay(100);
        digitalWrite(33,HIGH);
        delay(100);
    }
}

void myLed::on() {
        digitalWrite(33,LOW);
}

void myLed::off() {
        digitalWrite(33,HIGH);
}