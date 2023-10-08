#include "Arduino.h"
#include "LoRa_E220.h"

#define FCHANGE_TIMEOUT 2000
#define DELAY 100

void printData(struct RocketData packet);
void changeFrequencyLocal(unsigned f);
void changeFrequencyComplete(unsigned f);

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
          Serial.print("Attempting to change local and rocket frequency to ");
          Serial.println(f);
          changeFrequencyComplete(f);
          break;
        }
      case 'L':
        {
          unsigned f = Serial.parseInt();
          Serial.print("Changing local frequency to ");
          Serial.println(f);
          changeFrequencyLocal(f);
          break;
        }
    }
  }
  delay(DELAY);
}

void changeFrequencyLocal(unsigned freq) {
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

void changeFrequencyComplete(unsigned freq) {
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

  char msg[] = "F0";
  msg[1] = freq;

  ok = false;
  for(int counter = 0; counter < 10; counter++) {
    Serial.print("Attempt #");
    Serial.println(counter+1);
    for (int i = 0; i < 5; i++) {
      rs = e220ttl.sendMessage(msg);
      Serial.print(" > Send F packet #");
      Serial.print(i+1);
      Serial.print(": ");
      Serial.println(rs.getResponseDescription());
      delay(50);
    }
    config.CHAN = freq;
    Serial.println("Switching to new frequency");
    rs = e220ttl.setConfiguration(config, WRITE_CFG_PWR_DWN_SAVE);
    Serial.println(rs.getResponseDescription());
    long start = millis();
    do {
      Serial.println("Waiting for ack");
      delay(100);
      if (e220ttl.available()) {
        Serial.println("I received something");
        incoming = e220ttl.receiveMessage(sizeof(RocketData));
        packet = *(RocketData*)incoming.data;
        incoming.close();
        if (packet.code == 'C' || packet.code == 'D') {
          Serial.println("Sending ack back");
          rs = e220ttl.sendMessage("C");
          ok = true;
        }
      }
    } while (!ok && millis() - start < FCHANGE_TIMEOUT);

    if (ok) break;
    Serial.println("Switching back to old frequency");
    config.CHAN = old_freq;
    rs = e220ttl.setConfiguration(config, WRITE_CFG_PWR_DWN_SAVE);
    Serial.println(rs.getResponseDescription());
  }
  if (ok) Serial.println("Frequency change completed");
  else Serial.println("Could not change frequency in 10 attempts");
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
