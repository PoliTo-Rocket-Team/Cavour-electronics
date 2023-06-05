#include "Arduino.h"
#include "LoRa_E220.h"

#define DELAY 300
#define FCHANGE_TIMEOUT 5000
#define TX_LED 10
#define RX_LED 12

void handleData(struct RocketData packet);
void changeFrequency(unsigned f);

struct RocketData {
  char code;
  float bar;
  float temp;
  float ax;
  float ay;
  float az;
  float gx;
  float gy;
  float gz;
};

LoRa_E220 e220ttl(&Serial1, 2, 4, 6);  //  RX AUX M0 M1


void setup() {
  Serial.begin(9600);
  while(!Serial);
  e220ttl.begin();
  delay(500);
}

void loop() {
  if (e220ttl.available()) {
		ResponseStructContainer rsc = e220ttl.receiveMessageRSSI(sizeof(RocketData));
    struct RocketData packet = *(RocketData*) rsc.data;
    rsc.close();
    switch(packet.code) {
      case 'C':
      {
        e220ttl.sendMessage("C");
        break;
      }
      case 'D':
      {
        handleData(packet);
        break;
      }
      default:
        break;
    }
	}

  if (Serial.available()) {
    char cmd = Serial.read();
    switch(cmd) {
      case 'F': {
        unsigned f = Serial.parseInt();
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
  ResponseStatus rs;
  ResponseContainer incoming;
  bool ok;
  unsigned int old_freq;
  
  digitalWrite(TX_LED, HIGH);
  c = e220ttl.getConfiguration();
  config = *(Configuration*) c.data;
  c.close();
  old_freq = config.CHAN;

  char msg[] = "F0";
  msg[1] = freq;

  ok = false;
  while(1) {
    e220ttl.sendMessage(msg);
    config.CHAN = freq;
    rs = e220ttl.setConfiguration(config, WRITE_CFG_PWR_DWN_SAVE);
    long start = millis();
    do {
      delay(100);
      if(e220ttl.available()) {
        incoming = e220ttl.receiveMessage();
        if(incoming.data[0] == 'C' || incoming.data[0] == 'D'){
          e220ttl.sendMessage("C");
          ok = true;
        }
      }
    } while(!ok && millis() - start < FCHANGE_TIMEOUT);

    if(ok) break;

    config.CHAN = old_freq;
    rs = e220ttl.setConfiguration(config, WRITE_CFG_PWR_DWN_SAVE);
  }
  digitalWrite(TX_LED, LOW);
}

void handleData(struct RocketData packet) {
    Serial.print("\n");
    Serial.print("Pressure:");
    Serial.print(packet.bar);
    Serial.print("Temperature:");
    Serial.print(packet.temp);
    Serial.print("ax:");
    Serial.print(packet.ax);
    Serial.print("ay:");
    Serial.print(packet.ay);
    Serial.print("az:");
    Serial.print(packet.az);
    Serial.print("gx:");
    Serial.print(packet.gx);
    Serial.print("gy:");
    Serial.print(packet.gy);
    Serial.print("gz:");
    Serial.print(packet.gz);
    Serial.println();
}
