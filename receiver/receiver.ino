#include "Arduino.h"
#include "LoRa_E220.h"

#define DELAY 500
#define FCHANGE_TIMEOUT 5000

struct Messaggione {
  char code;
  byte bar[4];
  byte temp[4];
  byte acc_lx[4];
  byte acc_ly[4];
  byte acc_lz[4];
  byte acc_ax[4];
  byte acc_ay[4];
  byte acc_az[4];
};

LoRa_E220 e220ttl(&Serial1, 2, 4, 6);  //  RX AUX M0 M1


void handleData(ResponseStructContainer &msg);
void changeFrequency(long f);

void setup() {
  Serial.begin(9600);
  while(!Serial);
  e220ttl.begin();
  delay(500);
}

void loop() {
  if (e220ttl.available()) {
		ResponseStructContainer rsc = e220ttl.receiveMessageRSSI(sizeof(Messaggione));
    switch(rsc.data[0]) {
      case 'C':
      {
        e220ttl.sendMessage("C");
        break;
      }
      case 'D':
      {
        handleData(rsc);
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
        long freq = Serial.parseInt();
        changeFrequency(freq);
        break
      }
    }
  }
  delay(DELAY);
}

void changeFrequency(long f) {


  ResponseStructContainer c;
  c = e220ttl.getConfiguration();
  // It's important get configuration pointer before all other operation
  Configuration configuration = *(Configuration*) c.data;
  // TODO turn on led

  char msg[] = "F0";
  msg[1] = f;
  bool ok = false;
  ResponseContainer incoming;

  while(1) {
    e220ttl.sendMessage(msg);

    // TODO - change to new frequecy

    long start = millis();
    do {
      delay(250);
      if(e220ttl.available()) {
        incoming = e220ttl.receiveMessage();
        if(incoming.data[0] == 'C' || incoming.data[0] == 'D'){
          e220ttl.sendMessage("C");
          ok = true;
        }
      }
    } while(!ok && millis() - start < FCHANGE_TIMEOUT);

    if(ok) break;

    // TODO set old frequency
  }

  // TODO turn off led
}

void handleData(ResponseStructContainer &msg) {
  	struct Messaggione messaggione = *(Messaggione*) rsc.data.c_str();
    Serial.print("Pressure:");
    Serial.print( *(float*) messaggione.bar);
    Serial.print(",Temperature:");
    Serial.print( *(float*) messaggione.temp);
    Serial.print(",ax:");
    Serial.print( *(float*) messaggione.acc_lx);
    Serial.print(",ay:");
    Serial.print( *(float*) messaggione.acc_ly);
    Serial.print(",az:");
    Serial.print( *(float*) messaggione.acc_lz);
    Serial.print(",gx:");
    Serial.print( *(float*) messaggione.acc_ax);
    Serial.print(",gy:");
    Serial.print( *(float*) messaggione.acc_ay);
    Serial.print(",gz:");
    Serial.print( *(float*) messaggione.acc_az);
    // Serial.print(",RSSI:"); Serial.println(rsc.rssi, DEC); 
    Serial.println();
    rsc.close();
}
