#include "Arduino.h"
#include "LoRa_E220.h"

#define FCHANGE_TIMEOUT 2000
#define DELAY 100

void handleData(struct RocketData packet);
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

LoRa_E220 e220ttl(&Serial1, 2, 5, 7);  //  RX AUX M0 M1

ResponseStatus rs;
float reference;
bool reference_flag = true;


void setup() {
  Serial.begin(9600);
  while (!Serial)
    ;
  e220ttl.begin();
  Serial.println("Starting in 500 ms");
  delay(500);

  ResponseStructContainer c = e220ttl.getConfiguration();
  Configuration config = *(Configuration*)c.data;
  c.close();
  Serial.print("Old saved channel: ");
  Serial.println(config.CHAN);
  delay(200);
  Serial.println("Setting default frequency");
  config.CHAN = 76;
  delay(500);
  rs = e220ttl.setConfiguration(config, WRITE_CFG_PWR_DWN_SAVE);
  Serial.println(rs.getResponseDescription());
  Serial.println();
  delay(500);
}

void loop() {
  if (e220ttl.available()) {
    ResponseStructContainer rsc = e220ttl.receiveMessageRSSI(sizeof(RocketData));
    packet = *(RocketData*)rsc.data;
    rsc.close();
    Serial.println(packet.code);
    switch (packet.code) {
      case 'C':
        {
          rs = e220ttl.sendMessage("C");
          Serial.println(rs.getResponseDescription());
          break;
        }
      case 'D':
        {
          handleData(packet);
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

  char msg[] = "F0";
  msg[1] = freq;

  ok = false;
  int counter = 0;
  while (1) {
    for (int i = 0; i < 5; i++) {
      rs = e220ttl.sendMessage(msg);
      Serial.print("Attempt ");
      Serial.println(i);
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

    counter++;
    if (counter == 10) break;
  }
  if (ok) Serial.println("Frequency change completed");
  else Serial.println("Could not change frequency in 10 attempts");
}

void handleData(struct RocketData packet) {
  if (reference_flag) {
    reference = (packet.bar1+packet.bar2)*0.5;
    reference_flag = false;
  }
  
  float altitude = 44330 * (1.0 - pow((packet.bar1+packet.bar2)*0.5 / reference, 0.1903));
  Serial.print("\n");
  Serial.print("Altitude:");
  Serial.print(altitude);
  /*
  Serial.print(",Pressure1:");
  Serial.print(packet.bar1);
  Serial.print(",Pressure2:");
  Serial.print(packet.bar2);
  
  Serial.print(",Temperature1:");
  Serial.print(packet.temp1);
  Serial.print(",Temperature2:");
  Serial.print(packet.temp2);
  */

  
  Serial.print(",PressureAverage:");
  Serial.print((packet.bar1+packet.bar2)/2);

  Serial.print(",TemperatureAverage:");
  Serial.print((packet.temp1+packet.temp2)/2);

  
  Serial.print(",ax:");
  Serial.print(packet.ax);
  Serial.print(",ay:");
  Serial.print(packet.ay);
  Serial.print(",az:");
  Serial.print(packet.az);
  Serial.print(",gx:");
  Serial.print(packet.gx);
  Serial.print(",gy:");
  Serial.print(packet.gy);
  Serial.print(",gz:");
  Serial.print(packet.gz);
  Serial.println();
}

void printData(struct RocketData packet){
  
   float altitude = 44330 * (1.0 - pow((packet.bar1+packet.bar2)*0.5 / reference, 0.1903));
  Serial.print("\n");
  Serial.print("Altitude:");
  Serial.print(altitude);
  Serial.print(",PressureAverage:");
  Serial.print((packet.bar1+packet.bar2)/2);

  Serial.print(",TemperatureAverage:");
  Serial.print((packet.temp1+packet.temp2)/2);

  
  Serial.print(",ax:");
  Serial.print(packet.ax);
  Serial.print(",ay:");
  Serial.print(packet.ay);
  Serial.print(",az:");
  Serial.print(packet.az);
  Serial.print(",gx:");
  Serial.print(packet.gx);
  Serial.print(",gy:");
  Serial.print(packet.gy);
  Serial.print(",gz:");
  Serial.print(packet.gz);
}
  
