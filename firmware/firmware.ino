#include <BLEPeripheral.h>
#include "BLESerial.h"

BLESerial BLESerial;

const int led = 23;

void setup() {
    pinMode(led, OUTPUT);
    for (int i = 0; i < 9; i++) {
        digitalWrite(led, !digitalRead(led));
        delay(30);
    }

    // Custom UART test pins: 22 is actually the button...
    Serial.setPins(0, 22); // ...and RX is not used here
    Serial.begin(115200);
    Serial.println("Starting...");

    BLESerial.setLocalName("UART");
    BLESerial.begin();
}

void loop() {
    BLESerial.poll();

    forward();
    loopback();
    spam();
}


// forward received from Serial to BLESerial and vice versa
void forward() {
    if (BLESerial && Serial) {
        int byte;
        while ((byte = BLESerial.read()) > 0) Serial.write((char)byte);
        while ((byte = Serial.read()) > 0) BLESerial.write((char)byte);
    }
}

// echo all received data back
void loopback() {
    if (BLESerial) {
        int byte;
        while ((byte = BLESerial.read()) > 0) BLESerial.write(byte);
    }
}

// periodically sent time stamps
void spam() {
    if (BLESerial) {
        static int old = 0;
        int sec = millis() / 1000;

        if (old != sec) {
            old = sec;
            BLESerial.print(sec);
            digitalWrite(led, !digitalRead(led));
        }
    }
}
