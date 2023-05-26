#include <Arduino_LSM9DS1.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <SPI.h>
#include <SD.h>
#include "LoRa_E220.h"

#define SAMPLE_DELAY 50
#define SEND_TIMEOUT 500
#define COM_DELAY 100
#define OUTPUT_FILE "log.txt"
#define ERROR_LEDS 0

void read_P_T();
void changeFrequency(unsigned freq);

struct RocketData {
  float pressure;
  float temperature;
  float ax, ay, az;
  float gx, gy, gz;
} packet;

LoRa_E220 e220ttl(&Serial1, 2, 4, 6); //  RX AUX M0 M1
Adafruit_BMP280 bmp1;
Adafruit_BMP280 bmp2;
File myFile;

ResponseContainer incoming;

char data_line[140];

void setup() {
  Serial.begin(9600);
  delay(50);

  Serial1.begin(9600);
  e220ttl.begin();

#if ERROR_LEDS
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
#endif

  if (!SD.begin(A0)) {
    Serial.println("Errore di inizializzazione della scheda SD");
    return;
  }

  if (!IMU.begin()) {
    Serial.println("Errore IMU");
    while (1) {
      #if ERROR_LEDS
      digitalWrite(5, HIGH);
      delay(500);
      digitalWrite(5, LOW);
      #endif
      delay(500);
    }
  }

  if (!bmp1.begin(0x76)) {
    Serial.println("Errore bmp1");
    while (1) {
      #if ERROR_LEDS
      digitalWrite(6, HIGH);
      delay(500);
      digitalWrite(6, LOW);
      #endif
      delay(500);
    }
  }

  if (!bmp2.begin(0x77)) {
    Serial.println("Errore bmp2");
    while (1) {
      #if ERROR_LEDS
      digitalWrite(7, HIGH);
      delay(500);
      digitalWrite(7, LOW);
      #endif
      delay(500);
    }
  }

  myFile = SD.open(OUTPUT_FILE, FILE_WRITE);

  incoming.data = "A";
  while(incoming.data[0] != 'C') {
    if(e220ttl.available())
      incoming = e220ttl.receiveMessage();
    else {
      Serial.println("Send [C]");
      e220ttl.sendMessage("C");
    }
    delay(COM_DELAY);
  }
}

void loop() {
  static unsigned long last_send = millis();
  static unsigned long elapsed;

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
          Serial.println("Unrecognized command");
      }
    }
  }

  elapsed = last_send - millis();
  if(elapsed > SEND_TIMEOUT) {              // must send data
    last_send = millis();
    if(myFile) myFile.close();

    read_P_T();
    // send new data if possible, old otherwise
    bool old = true;
    if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
      IMU.readAcceleration(packet.ax,packet.ay,packet.az);
      IMU.readGyroscope(packet.gx,packet.gy,packet.gz);
      old = false;
    }

    // send code  

    char str[2 + sizeof(RocketData)];
    str[0] = 'D';
    str[1 + sizeof(RocketData)] = '\0';
    memcpy((&str) + 1, (void*) &packet, sizeof(RocketData));
    ResponseStatus rs = e220ttl.sendMessage(str);
    Serial.print("Data sent");

    if(myFile = SD.open(OUTPUT_FILE, FILE_WRITE)) { // if file can be opened, save
      sprintf(data_line, "%u,%.6f,%.6f,NaN,NaN,NaN,NaN,NaN,NaN", millis(), packet.pressure, packet.temperature);
      sprintf(data_line, "%u,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f", millis(), packet.pressure, packet.temperature, 
                                                                       packet.ax, packet.ay, packet.az, 
                                                                       packet.gx, packet.gy, packet.gz);
      myFile.println(data_line);
    }
  }
  else {                                    // write to SD
    // try once to open file
    if(!myFile) 
      if(!(myFile = SD.open(OUTPUT_FILE, FILE_WRITE)))
        return; // do not even read data if file is not available
    // surely here the file is open
    read_P_T();
    if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
      IMU.readAcceleration(packet.ax,packet.ay,packet.az);
      IMU.readGyroscope(packet.gx,packet.gy,packet.gz);
      sprintf(data_line, "%u,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f", millis(), packet.pressure, packet.temperature, 
                                                                       packet.ax, packet.ay, packet.az, 
                                                                       packet.gx, packet.gy, packet.gz);
    }
    else {
      sprintf(data_line, "%u,%.6f,%.6f,NaN,NaN,NaN,NaN,NaN,NaN", millis(), packet.pressure, packet.temperature);
    }
    myFile.println(data_line);
  }

  delay(SAMPLE_DELAY); 
}


void read_P_T() {
  packet.pressure = (bmp1.readPressure() + bmp2.readPressure()) * 0.5e-2;
  packet.temperature = (bmp1.readTemperature() + bmp2.readTemperature()) * 0.5;
}


void changeFrequency(unsigned freq) {
  ResponseStructContainer c;
  c = e220ttl.getConfiguration();
  // It's important get configuration pointer before all other operation
  Configuration configuration = *(Configuration*) c.data;
  Serial.println(c.status.getResponseDescription());
  Serial.println(c.status.code);

  Serial.print("Mi hai detto che la frequenza e' \t");
  Serial.println(freq);

  // printParameters(configuration);

  ResponseStatus outgoing;
  incoming.data = "A";
  do {
    outgoing = e220ttl.sendMessage("C");
    Serial.println(outgoing.getResponseDescription());
    delay(COM_DELAY);

    if(!e220ttl.available()) continue;

    incoming = e220ttl.receiveMessage();
    Serial.print("Message received -> ");   // LOG - TO BE ELIMINATED
    if (incoming.status.code!=1) {
      Serial.print("error: ");
      Serial.println(incoming.status.getResponseDescription());
    }
    else {  // print received data
      Serial.print("raw: ");
      Serial.println(incoming.data);
    }
  } while(incoming.data[0] != 'C');
}
