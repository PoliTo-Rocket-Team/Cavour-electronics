#include "Arduino.h"
#include "LoRa_E220.h"

#define DELAY 100

void printData(struct RocketData packet);
void changeFrequency(unsigned f);

struct RocketData {
  char code;
  float bar1, bar2;
  float temp1, temp2;
  float ax;
  float ay;
  float az;
  float gx;
  float gy;
  float gz;
} packet;

LoRa_E220 e220ttl(3, 2, 5, 8, 10);

ResponseStatus rs;
float reference;
bool reference_flag = true;
unsigned int rssi;


void setup() {
  Serial.begin(9600);
  while (!Serial);
  while (!e220ttl.begin());
  Serial.println("Starting...");

 ResponseStructContainer c = e220ttl.getConfiguration();
 Configuration config = *(Configuration*)c.data;
 c.close();
 Serial.print("Old saved channel: ");
 Serial.println(config.CHAN);
}

void loop() {
  if (e220ttl.available()) {
    ResponseStructContainer rsc = e220ttl.receiveMessageRSSI(sizeof(RocketData));
    rssi = rsc.rssi;
    packet = *(RocketData*)rsc.data;
    rsc.close();
    switch (packet.code) {
      case 'C':
        {
          rs = e220ttl.sendMessage("C");
          Serial.println(rs.getResponseDescription());
          break;
        }
      case 'D':
        {
          printData(packet);
          break;
        }
      default:
        break;
    }
  }

  if (Serial.available()) {
    char cmd = Serial.read();
    switch (cmd) {
      case 'F':
        {
          unsigned f = Serial.parseInt();
          Serial.print("Changing frequency: ");
          Serial.println(f);
          changeFrequency(f);
          break;
        }
    }
  }
  delay(DELAY);
}

void changeFrequency(unsigned freq) {
  ResponseStructContainer c;
  Configuration config;
  ResponseStructContainer incoming;
  bool ok;
  unsigned int old_freq;

  c = e220ttl.getConfiguration();
  config = *(Configuration*)c.data;
  c.close();
  old_freq = config.CHAN;
  Serial.print("Old frequency: ");
  Serial.println(old_freq);

  config.CHAN = freq;
  Serial.println("Switching to new frequency");
  rs = e220ttl.setConfiguration(config, WRITE_CFG_PWR_DWN_SAVE);
  Serial.println(rs.getResponseDescription());
}

void handleData(struct RocketData packet) {
  if (reference_flag) {
    reference = (packet.bar1+packet.bar2)*0.5;
    reference_flag = false;
  }
}

void printData(struct RocketData packet) {
  if (reference_flag) {
    reference = (packet.bar1+packet.bar2)*0.5;
    reference_flag = false;
  }

  float altitude = 44330 * (1.0 - pow((packet.bar1+packet.bar2)*0.5 / reference, 0.1903));

  Serial.print("RSSI:");
    Serial.println(rssi);
  Serial.print(",Altitude:");
    Serial.println(altitude);
  Serial.print(",PressureAverage:");
    Serial.println((packet.bar1+packet.bar2)/2);
  Serial.print(",TemperatureAverage:");
    Serial.println((packet.temp1+packet.temp2)/2);
  Serial.print(",ax:");
    Serial.println(packet.ax);
  Serial.print(",ay:");
    Serial.println(packet.ay);
  Serial.print(",az:");
    Serial.println(packet.az);
  }
