#include "Arduino.h"
#include "LoRa_E220.h"

#define TX_LED 10
#define RX_LED 12
#define FCHANGE_TIMEOUT 2000
#define DELAY 100

void hadleData(struct RocketData packet);
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
} packet;

LoRa_E220 e220ttl(&Serial1, 2, 4, 6);  //  RX AUX M0 M1

ResponseStatus rs;


void setup() {
  Serial.begin(9600);
  while (!Serial);
  e220ttl.begin();
  Serial.println("Starting in 500 ms");
  delay(500);

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
  
  digitalWrite(TX_LED, HIGH);
  c = e220ttl.getConfiguration();
  config = *(Configuration*) c.data;
  c.close();
  old_freq = config.CHAN;
  Serial.print("Old frequency: ");
  Serial.println(old_freq);

  char msg[] = "F0";
  msg[1] = freq;

  ok = false;
  while(1) {
    rs = e220ttl.sendMessage(msg);
    Serial.println(rs.getResponseDescription());
    config.CHAN = freq;
    Serial.println("Switching to new frequency");
    rs = e220ttl.setConfiguration(config, WRITE_CFG_PWR_DWN_SAVE);
    Serial.println(rs.getResponseDescription());
    long start = millis();
    do {
      Serial.println("Waiting for ack");
      delay(100);
      if(e220ttl.available()) {
        Serial.println("I received something");
        incoming = e220ttl.receiveMessage(sizeof(RocketData));
        packet = *(RocketData*)incoming.data;
        incoming.close();
        if(packet.code == 'C' || packet.code == 'D'){
          Serial.println("Sending ack back");
          rs = e220ttl.sendMessage("C");
          ok = true;
        }
      }
    } while(!ok && millis() - start < FCHANGE_TIMEOUT);

    if(ok) break;
    Serial.println("Switching back to old frequency");
    config.CHAN = old_freq;
    rs = e220ttl.setConfiguration(config, WRITE_CFG_PWR_DWN_SAVE);
    Serial.println(rs.getResponseDescription());
  }
  digitalWrite(TX_LED, LOW);
  Serial.println("Frequency change completed");
}

void handleData(struct RocketData packet) {
    Serial.print("\n");
    Serial.print("Pressure:");
    Serial.print(packet.bar);
    Serial.print(",Temperature:");
    Serial.print(packet.temp);
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
