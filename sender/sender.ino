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
#define TX_LED 5
#define RX_LED 6
#define SD_LED 7

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

float ax, ay, az;
float gx, gy, gz;
float bar, temp;

char data_line[140];

LoRa_E220 e220ttl(&Serial1, 4, 2, 3); //  RX AUX M0 M1
Adafruit_BMP280 bmp1;
Adafruit_BMP280 bmp2;

File myFile;
ResponseContainer incoming;
ResponseStatus rs;


void setup() {
  pinMode(TX_LED, OUTPUT);
  pinMode(RX_LED, OUTPUT);
  pinMode(SD_LED, OUTPUT);

  Serial.begin(9600);   // LOG - TO BE ELIMINATED 
  while(!Serial);

  Serial1.begin(9600);
  
  if (!e220ttl.begin()) {
    digitalWrite(TX_LED, HIGH);
    digitalWrite(RX_LED, HIGH);
    digitalWrite(SD_LED, HIGH);
    while (1);
  }

  if (!SD.begin(A0)) {
    Serial.println("Errore di inizializzazione della scheda SD");   // LOG - TO BE ELIMINATED
    while (1) {
      digitalWrite(SD_LED, HIGH);
      delay(1000);
      digitalWrite(SD_LED, LOW);
      delay(1000);
    }
  }
  
  if (!IMU.begin()) {
    Serial.println("Errore IMU");   // LOG - TO BE ELIMINATED
    while (1) {
      digitalWrite(SD_LED, HIGH);
      delay(200);
      digitalWrite(SD_LED, LOW);
      delay(200);
    }
  }

  if (!bmp1.begin(0x76)) {
    Serial.println("Errore bmp1");   // LOG - TO BE ELIMINATED
    while (1) {
      digitalWrite(SD_LED, HIGH);
      delay(2000);
      digitalWrite(SD_LED, LOW);
      delay(2000);
    }
  }

  if (!bmp2.begin(0x77)) {
    Serial.println("Errore bmp2");   // LOG - TO BE ELIMINATED
    while (1) {
      digitalWrite(SD_LED, HIGH);
      delay(3000);
      digitalWrite(SD_LED, LOW);
      delay(3000);
    }
  }

  if(!(myFile = SD.open(OUTPUT_FILE, FILE_WRITE))) {
    Serial.println("Could not open file in setup :(");    // LOG - TO BE ELIMINATED 
    while(1) {
      digitalWrite(SD_LED, HIGH);
      delay(4000);
      digitalWrite(SD_LED, LOW);
      delay(4000);
    }
  }

  ResponseStructContainer c = e220ttl.getConfiguration();
  Configuration config = *(Configuration*) c.data;
  c.close();
  Serial.print("Old saved channel: ");   // LOG - TO BE ELIMINATED
  Serial.println(config.CHAN);   // LOG - TO BE ELIMINATED
  delay(200);   // LOG - TO BE ELIMINATED
  Serial.println("Setting default frequency");   // LOG - TO BE ELIMINATED
  config.CHAN = 23;
  delay(500);
  rs = e220ttl.setConfiguration(config, WRITE_CFG_PWR_DWN_SAVE);
  Serial.println(rs.getResponseDescription());   // LOG - TO BE ELIMINATED
  Serial.println();   // LOG - TO BE ELIMINATED
  delay(500);

  incoming.data = "A";
  while(incoming.data[0] != 'C') {
    if(e220ttl.available())
      incoming = e220ttl.receiveMessage();
      Serial.print("I received something:");   // LOG - TO BE ELIMINATED
      Serial.println(incoming.data);   // LOG - TO BE ELIMINATED
    else {  
      packet.code = 'C';
      rs = e220ttl.sendMessage(&packet, sizeof(RocketData));
      Serial.println("Transmitting check");   // LOG - TO BE ELIMINATED
      Serial.println(rs.getResponseDescription());   // LOG - TO BE ELIMINATED
    }
    delay(COM_DELAY);
  }
}

void loop() {
  static unsigned long last_send = millis();
  static unsigned long elapsed;
  bool old;

  if(e220ttl.available()) {
    Serial.print("Message received -> ");   // LOG - TO BE ELIMINATED
    incoming = e220ttl.receiveMessage();
    if (incoming.status.code != 1){
      Serial.print("error: "); 
      Serial.println(incoming.status.getResponseDescription());
    } else {  // print received data
      Serial.print("raw: ");
      Serial.println(incoming.data);
      switch(incoming.data[0]) {
        case 'F': 
          changeFrequency(incoming.data[1]);
          break;
        default:
          Serial.println("Unrecognized command");   // LOG - TO BE ELIMINATED
      }
    }
  }

  old = true;
  readPT();
  if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
    readAG();
    old = false;
  }
  if (old) sprintf(data_line, "%u,%.6f,%.6f,NaN,NaN,NaN,NaN,NaN,NaN", millis(), bar, temp);
  else sprintf(data_line, "%u,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f", millis(), bar, temp, ax, ay, az, gx, gy, gz);

  if(myFile) {
    myFile.println(data_line);
    Serial.println("Data saved to SD");   // LOG - TO BE ELIMINATED 
  } else Serial.println("Could not open file");   // LOG - TO BE ELIMINATED

  // if SEND_TIMEOUT elapsed, send data and flush SD 
  elapsed = last_send - millis();
  if (elapsed > SEND_TIMEOUT) {
    last_send = millis();
    if (myFile) {
      myFile.flush();
      Serial.println("SD flushed");   // LOG - TO BE ELIMINATED
    } 
    sendData();
    Serial.println("Data transmitted");   // LOG - TO BE ELIMINATED
  }
  delay(SAMPLE_DELAY); 
}


void readPT() {
  bar = (bmp1.readPressure() + bmp2.readPressure()) * 0.5e-2;
  temp = (bmp1.readTemperature() + bmp2.readTemperature()) * 0.5;
}

void readAG() {
  IMU.readAcceleration(ax, ay, az);
  IMU.readGyroscope(gx, gy, gz);
}

void sendData() {
  packet.code = 'D';
  *(float*) packet.bar = bar;
  *(float*) packet.temp = temp;
  *(float*) packet.ax = ax;
  *(float*) packet.ay = ay;
  *(float*) packet.az = az;
  *(float*) packet.gx = gx;
  *(float*) packet.gy = gy;
  *(float*) packet.gz = gz;
  ResponseStatus rs = e220ttl.sendMessage(&packet, sizeof(RocketData));
  Serial.println(rs.getResponseDescription());   // LOG - TO BE ELIMINATED
}

void changeFrequency(unsigned freq) {
  ResponseStructContainer c = e220ttl.getConfiguration();
  Configuration config = *(Configuration*) c.data;
  c.close();
  config.CHAN = freq;
  ResponseStatus rs = e220ttl.setConfiguration(config, WRITE_CFG_PWR_DWN_SAVE);

  ResponseStatus outgoing;
  incoming.data = "A";
  do {
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
}
