#include "LoRa_E220.h"

#define FCHANGE_TIMEOUT 2000

LoRa_E220 e220ttl(&Serial1, 2, 5, 7);  //  RX AUX M0 M1
// LoRa_E220 e220ttl(3, 2, 5, 8, 10);

bool backend_connected = false;
byte frequency = 0xFF;

void changeFrequency(byte);

struct RocketData {
  char code;
  float bar1, bar2;
  float temp1, temp2;
  float al_x;
  float al_y;
  float al_z;
  float aa_x;
  float aa_y;
  float aa_z;
} packet;


void setup() {
  randomSeed(analogRead(0));
  Serial.begin(9600);
  while (!Serial);
  while (!e220ttl.begin());
  delay(500);
}

void loop() {
  if (Serial.available()) {
    char inChar;
    Serial.readBytes(&inChar, 1);
    switch (inChar) {
      case 'G':
        backend_connected = true;
        break;
      case 'L': {
        byte new_frequency;
        Serial.readBytes(&new_frequency, 1);
        ResponseStructContainer c = e220ttl.getConfiguration();
        Configuration configuration = *((Configuration *)c.data);
        // configuration.ADDL = 0x03;
        // configuration.ADDH = 0x00;
        configuration.CHAN = frequency = new_frequency;
        e220ttl.setConfiguration(configuration, WRITE_CFG_PWR_DWN_SAVE);
        c.close();
        Serial.write("~G", 2);
        break;
      }
      case 'F':
        {
          // complete procedure
          byte new_frequency;
          Serial.readBytes(&new_frequency, 1);
          changeFrequency(new_frequency);
          break;
        }
    }
  }

  if (!backend_connected) {
    // Sending Ready Signal to Backend
    Serial.write("~G",2);
    delay(250);
    return;
  }

  if (e220ttl.available() > 0) {

    ResponseStructContainer rsc = e220ttl.receiveMessageRSSI(sizeof(RocketData));
    packet = *(RocketData*)rsc.data;
    rsc.close();
    switch (packet.code) {
      case 'C':
        {
          delay(10);
          e220ttl.sendMessage("CCCCC");
          Serial.write("~C",2);
          break;
        }
      case 'D':
        {
          Serial.write('~');
          Serial.write((byte*) &packet, sizeof(RocketData));
          break;
        }
      default:
        break;
    }
  }
  delay(100);
}

float randomFloat(float minf, float maxf) {
  return minf + random(1UL << 31) * (maxf - minf) / (1UL << 31);  // use 1ULL<<63 for max double values)
}

void changeFrequency(byte new_freq) {
  ResponseStructContainer c;
  Configuration config;
  ResponseStructContainer incoming;
  bool ok;

  c = e220ttl.getConfiguration();
  config = *(Configuration*)c.data;
  c.close();
  unsigned old_freq = config.CHAN;
  // unsigned int old_freq = frequency;

  char msg[] = "F0";
  msg[1] = new_freq;

  ok = false;
  int counter = 0;
  while (counter < 10) {
    for (int i = 0; i < 5; i++) {
      e220ttl.sendMessage(msg);
      delay(50);
    }
    config.CHAN = new_freq;
    e220ttl.setConfiguration(config, WRITE_CFG_PWR_DWN_SAVE);
    long start = millis();
    do {
      // Waiting for ack
      delay(100);
      if (e220ttl.available()) {
        // I received something 
        incoming = e220ttl.receiveMessage(sizeof(RocketData));
        packet = *(RocketData*)incoming.data;
        incoming.close();
        if (packet.code == 'C' || packet.code == 'D') {
          e220ttl.sendMessage("CCCCC");
          Serial.write("~C",2);
          ok = true;
        }
      }
    } while (!ok && millis() - start < FCHANGE_TIMEOUT);

    if (ok) break;
    // switching back to old frequency
    // config.ADDL = 0x03;
	  // config.ADDH = 0x00;
    config.CHAN = old_freq;
    e220ttl.setConfiguration(config, WRITE_CFG_PWR_DWN_SAVE);
    counter++;
  }
  if(!ok) Serial.write("~R",2);
  frequency = ok ? new_freq : 0xFF;
}
