#include <Arduino_LSM9DS1.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include "LoRa_E220.h"
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

#define SAMPLE_DELAY 50
#define SEND_TIMEOUT 500
#define COM_DELAY 300
#define OUTPUT_FILE "log.txt"

void readPT();
void readAG();
void sendData();
void changeFrequency(unsigned freq);

struct RocketData {
  char code;
  byte bar[4];
  byte temp[4];
  byte ax[4];
  byte ay[4];
  byte az[4];
  byte gx[4];
  byte gy[4];
  byte gz[4];
} packet;

LoRa_E220 e220ttl(&Serial1, 4, 2, 3);  //  RX AUX M0 M1
Adafruit_BMP280 bmp1;
Adafruit_BMP280 bmp2;

ResponseContainer incoming;
ResponseStatus rs;

File myFile;
ResponseContainer incoming;
ResponseStatus rs;

float ax, ay, az;
float gx, gy, gz;
float bar, temp;
char data_line[140];


void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial1.begin(9600);

  if (!e220ttl.begin()) {
    Serial.println("Could not initialize LoRa module :(");
    while(1);
  }

  if (!SD.begin(A0)) {
    Serial.println("Could not initialize SD card :(");
    while(1);
  }

  if (!IMU.begin()) {
    Serial.println("Errore IMU");
    while (1);
  }

  if (!bmp1.begin(0x76)) {
    Serial.println("Errore bmp1");
    while (1);
  }

  if (!bmp2.begin(0x77)) {
    Serial.println("Errore bmp2");
    while (1);
  }

  if(!(myFile = SD.open(OUTPUT_FILE, FILE_WRITE))) {
    Serial.println("Could not open file :(");
    while(1);
  }

  ResponseStructContainer c = e220ttl.getConfiguration();
  Configuration config = *(Configuration*) c.data;
  c.close();
  Serial.print("Old saved channel: ");
  Serial.println(config.CHAN);
  delay(200);
  Serial.println("Setting default frequency");
  config.CHAN = 23;
  delay(500);
  rs = e220ttl.setConfiguration(config, WRITE_CFG_PWR_DWN_SAVE);
  Serial.println(rs.getResponseDescription());
  Serial.println();
  delay(500);

  incoming.data = "A";
  while (incoming.data[0] != 'C') {
    if (e220ttl.available()) {
      incoming = e220ttl.receiveMessage();
      Serial.print("I received something:");
      Serial.println(incoming.data);
    }
    else {
      packet.code = 'C';
      rs = e220ttl.sendMessage(&packet, sizeof(RocketData));
      Serial.println("Transmitting check");
      Serial.println(rs.getResponseDescription());
    }
    delay(COM_DELAY);
  }
}

void loop() {
  static unsigned long last_send = millis();
  static unsigned long elapsed;
  bool old;

  if (e220ttl.available()) {
    Serial.print("Message received -> ");  // LOG - TO BE ELIMINATED
    incoming = e220ttl.receiveMessage();
    if (incoming.status.code != 1) {
      Serial.print("error: ");
      Serial.println(incoming.status.getResponseDescription());
    } else {  // print received data
      Serial.print("raw: ");
      Serial.println(incoming.data);
      switch (incoming.data[0]) {
        case 'F':
          changeFrequency(incoming.data[1]);
          break;
        default:
          Serial.println("Unrecognized command");  // LOG - TO BE ELIMINATED
      }
    }
  }

  old = true;
  if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
    readAG();
    old = false;
  }
  readPT();
  
  if (old) sprintf(data_line, "%u,%.6f,%.6f,NaN,NaN,NaN,NaN,NaN,NaN", millis(), bar, temp);
  else sprintf(data_line, "%u,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f", millis(), bar, temp, ax, ay, az, gx, gy, gz);

  if (myFile) {
    myFile.println(data_line);
    Serial.println("Data written to SD card");
  } else Serial.println("File is not open!");
  
  elapsed = last_send - millis();
  if (elapsed > SEND_TIMEOUT) {
    last_send = millis();
    if (myFile) {
      myFile.flush();
      Serial.println("SD card flushed");
    }
    sendData();
  }
  
  delay(SAMPLE_DELAY);
}

void sendData() {
  packet.code = 'D';
  *(float*)packet.ax = ax;
  *(float*)packet.ay = ay;
  *(float*)packet.az = az;
  *(float*)packet.gx = gx;
  *(float*)packet.gy = gy;
  *(float*)packet.gz = gz;
  *(float*)packet.bar = bar;
  *(float*)packet.temp = temp;

  ResponseStatus rs = e220ttl.sendMessage(&packet, sizeof(RocketData));
  Serial.println("Transmitting data");
  Serial.println(rs.getResponseDescription());
}

void readAG() {
  IMU.readAcceleration(ax, ay, az);
  IMU.readGyroscope(gx, gy, gz);
}

void readPT() {
  bar = (bmp1.readPressure()+bmp2.readPressure())*0.5e-2;
  temp = (bmp1.readTemperature()+bmp2.readTemperature())*0.5;
}

void changeFrequency(unsigned freq) {
  ResponseStructContainer c = e220ttl.getConfiguration();
  Configuration config = *(Configuration*) c.data;
  c.close();
  config.CHAN = freq;
  Serial.println("Switchig to new frequency");
  ResponseStatus rs = e220ttl.setConfiguration(config, WRITE_CFG_PWR_DWN_SAVE);

  ResponseStatus outgoing;
  incoming.data = "A";
  do {
    Serial.println("Sending ack");
    packet.code = 'C';
    outgoing = e220ttl.sendMessage(&packet, sizeof(RocketData));
    Serial.println(outgoing.getResponseDescription());   // LOG - TO BE ELIMINATED
    delay(COM_DELAY);

    // LOG - TO BE ELIMINATED
    if(!e220ttl.available()) continue;

    incoming = e220ttl.receiveMessage();
    Serial.print("Message received -> ");
    if (incoming.status.code!=1) {
      Serial.print("error: ");
      Serial.println(incoming.status.getResponseDescription());
    }
    else {  // print received data
      Serial.print("raw: ");
      Serial.println(incoming.data);
    }
    // LOG - TO BE ELIMINATED
  } while(incoming.data[0] != 'C');
  Serial.println("Frequency change completed");
}
