#include <BLEPeripheral.h>
#include "BLESerial.h"

#include <Wire.h>
#include <BMI160Gen.h>

BLESerial BLESerial;

const int led = 23;

void setup() {
    // initialize LEDs
    pinMode(led, OUTPUT);
    for (int i = 0; i < 9; i++) {
        digitalWrite(led, !digitalRead(led));
        delay(60);
    }

    // initialize UART
    Serial.setPins(6, 12); // RX is not used here (6 is a random unused pin)
    Serial.begin(115200);
    while (!Serial); // wait...
    Serial.println("Starting...");

    // initialize BLE
    BLESerial.setLocalName("UART");
    BLESerial.begin();

    // initialize IMU
    pinMode(29 , OUTPUT);  // pin 29 = SDO...
    digitalWrite(29, LOW); // ...to select default I2C address
    Wire.setPins(30, 0);   // SDA, SCL
    BMI160.begin(BMI160GenClass::I2C_MODE);
}

void loop() {
    BLESerial.poll();

    forward();
    loopback();
    sendIMUdata();
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

// periodically sent IMU data
void sendIMUdata() {
    static int old_ms = 0;
    const long period_ms = 100;
    long now_ms = millis();

    if (now_ms >= old_ms + period_ms) {
        old_ms = now_ms;

        int axRaw, ayRaw, azRaw, gxRaw, gyRaw, gzRaw; // raw values
        float ax, ay, az, gx, gy, gz;

        // read raw gyro measurements from device
        BMI160.readMotionSensor(axRaw, ayRaw, azRaw, gxRaw, gyRaw, gzRaw);
        now_ms = millis(); // refresh

        // convert the raw accel data to g
        ax = convertRawAccel(axRaw);
        ay = convertRawAccel(ayRaw);
        az = convertRawAccel(azRaw);
        // convert the raw gyro data to degrees/second
        gx = convertRawGyro(gxRaw);
        gy = convertRawGyro(gyRaw);
        gz = convertRawGyro(gzRaw);

        // display values
        Serial.print("t:\t");   Serial.print(now_ms);

        Serial.print("\ta:\t"); Serial.print(ax);
        Serial.print("\t");     Serial.print(ay);
        Serial.print("\t");     Serial.print(az);

        Serial.print("\tg:\t"); Serial.print(gx);
        Serial.print("\t");     Serial.print(gy);
        Serial.print("\t");     Serial.print(gz);
        Serial.println();

        if (BLESerial) {
            BLESerial.print("t:\t");   BLESerial.print(now_ms);

            BLESerial.print("\ta:\t"); BLESerial.print(ax);
            BLESerial.print("\t");     BLESerial.print(ay);
            BLESerial.print("\t");     BLESerial.print(az);

            BLESerial.print("\tg:\t"); BLESerial.print(gx);
            BLESerial.print("\t");     BLESerial.print(gy);
            BLESerial.print("\t");     BLESerial.print(gz);
            BLESerial.println();
        }
        digitalWrite(led, !digitalRead(led));
    }
}

float convertRawAccel(int aRaw) {
  // we use the +/- 2g range (maps to a raw value of +/- 2**15)
  float a = (aRaw * 2.0) / (1<<15);
  return a;
}

float convertRawGyro(int gRaw) {
  // we use the +/- 250 degrees/seconds range (maps to +/- 2**15)
  float g = (gRaw * 250.0) / (1<<15);
  return g;
}

