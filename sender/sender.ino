#include <ReefwingLSM9DS1.h> // https://github.com/Reefwing-Software/Reefwing-LSM9DS1/tree/main
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <SPI.h>
#include <SD.h>
#include "LoRa_E220.h"

// #define LSM9DS1AG_ADDRESS 0x6B  // Address of accelerometer & gyroscope
// #define LSM9DS1M_ADDRESS  0x1E  // Address of magnetometer
#define OUTPUT_FILE "log.txt"
#define SAMPLE_DELAY 50
#define SEND_TIMEOUT 1000
#define COM_DELAY 300
#define TX_LED 5
#define RX_LED 6
#define SD_LED 7

void changeFrequency(unsigned freq);
void readAG();
void readPT();
void sendData();

struct RocketData {
  char code;
  byte bar1[4], bar2[4];
  byte temp1[4], temp2[4];
  byte ax[4];
  byte ay[4];
  byte az[4];
  byte gx[4];
  byte gy[4];
  byte gz[4];
} packet;

Adafruit_BMP280 bmp1;
Adafruit_BMP280 bmp2;
ReefwingLSM9DS1 imu; // new

LoRa_E220 e220ttl(&Serial1, 4, 2, 3);  //  RX AUX M0 M1

// Configuration config;
File myFile;
ResponseContainer incoming;
ResponseStatus rs;

float ax, ay, az;
float gx, gy, gz;
float bar1, bar2;
float temp1, temp2;
char data_line[160];
static unsigned long last_send;
static unsigned long elapsed;
bool old;


void setup() {
  pinMode(TX_LED, OUTPUT);
  pinMode(RX_LED, OUTPUT);
  pinMode(SD_LED, OUTPUT);

  Serial1.begin(9600);
  //while(!e220ttl);
  delay(500);
  if (!e220ttl.begin()) {
    digitalWrite(TX_LED, HIGH);
    digitalWrite(RX_LED, HIGH);
    digitalWrite(SD_LED, HIGH);
    while (1);
  }
//
//  if (!SD.begin(A0)) {
//    while (1) {
//      digitalWrite(SD_LED, HIGH);
//      delay(1000);
//      digitalWrite(SD_LED, LOW);
//      delay(1000);
//    }
//  }

  if (!imu.begin()) {
    while (1) {
      digitalWrite(SD_LED, HIGH);
      delay(200);
      digitalWrite(SD_LED, LOW);
      delay(200);
    }

    // initial configuration
    imu.start();
    imu.setAccelScale(FS_XL_16G); // extend range to +-16G
    imu.calibrateGyro();
    imu.calibrateAccel();
    imu.calibrateMag();

    delay(20);
    //  Flush the first reading - this is important!
    //  Particularly after changing the configuration.
    imu.readGyro();
    imu.readAccel();
    imu.readMag();
  }

  if (!bmp1.begin(0x76)) {
    while (1) {
      digitalWrite(SD_LED, HIGH);
      delay(2000);
      digitalWrite(SD_LED, LOW);
      delay(2000);
    }
  }

  if (!bmp2.begin(0x77)) {
    while (1) {
      digitalWrite(SD_LED, HIGH);
      delay(3000);
      digitalWrite(SD_LED, LOW);
      delay(3000);
    }
  }
//
//  if(!(myFile = SD.open(OUTPUT_FILE, FILE_WRITE))) {
//    while(1) {
//      digitalWrite(SD_LED, HIGH);
//      delay(4000);
//      digitalWrite(SD_LED, LOW);
//      delay(4000);
//    }
//  }
//
//  ResponseStructContainer c = e220ttl.getConfiguration();
//  Configuration config = *(Configuration*)c.data;
//  c.close();
//  config.CHAN = 23;
//   delay(500);
//  rs = e220ttl.setConfiguration(config, WRITE_CFG_PWR_DWN_SAVE);
//  delay(500);

  incoming.data = "A";
  while (incoming.data[0] != 'C') {
    if (e220ttl.available()) {
      incoming = e220ttl.receiveMessage();
    } else {
      packet.code = 'C';
      rs = e220ttl.sendMessage(&packet, sizeof(RocketData));
    }
    delay(COM_DELAY);
  }
  last_send = millis();
}

void loop() {

  if (e220ttl.available()) {
    incoming = e220ttl.receiveMessage();
    switch (incoming.data[0]) {
      case 'F':
        changeFrequency(incoming.data[1]);
        break;
      default:
        break;
    }
  }
  
  readAG();
  readPT();

  sprintf(data_line, "%u,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f", millis(), bar1, bar2, temp1, temp2, ax, ay, az, gx, gy, gz);

  if (myFile) {
    digitalWrite(SD_LED, HIGH);
    myFile.println(data_line);
    digitalWrite(SD_LED, LOW);
  }
  
  elapsed = millis() - last_send;
  if (elapsed > SEND_TIMEOUT) {
    last_send = millis();
    if (myFile) myFile.flush();
    digitalWrite(TX_LED, HIGH);
    sendData();
    digitalWrite(TX_LED, LOW);
  }
  
  delay(SAMPLE_DELAY);
}


void readPT() {
  bar1 = bmp1.readPressure()*0.01; 
  bar2 = bmp2.readPressure()*0.01;
  temp1 = bmp1.readTemperature();
  temp2 = bmp2.readTemperature();
}

void readAG() {
  imu.updateSensorData();
  ax = imu.data.ax;
  ay = imu.data.ay;
  az = imu.data.az;
  gx = imu.data.gx;
  gy = imu.data.gy;
  gz = imu.data.gz;
  // IMU.readAcceleration(ax, ay, az);
  // IMU.readGyroscope(gx, gy, gz);
}

void sendData() {
  packet.code = 'D';
  *(float*)packet.ax = ax;
  *(float*)packet.ay = ay;
  *(float*)packet.az = az;
  *(float*)packet.gx = gx;
  *(float*)packet.gy = gy;
  *(float*)packet.gz = gz;
  *(float*)packet.bar1 = bar1;
  *(float*)packet.bar2 = bar2;
  *(float*)packet.temp1 = temp1;
  *(float*)packet.temp2 = temp2;

  ResponseStatus rs = e220ttl.sendMessage(&packet, sizeof(RocketData));
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
    delay(COM_DELAY);

    if(e220ttl.available()) incoming = e220ttl.receiveMessage();

  } while(incoming.data[0] != 'C');
}
