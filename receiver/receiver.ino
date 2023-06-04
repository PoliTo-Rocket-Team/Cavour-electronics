#include "Arduino.h"
#include "LoRa_E220.h"

#define DELAY 500
#define FCHANGE_TIMEOUT 5000
#define TX_LED 10
#define RX_LED 12

void handleData(ResponseStructContainer &msg);
void changeFrequency(long f);

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



void setup() {
  Serial.begin(9600);
  while(!Serial);
  e220ttl.begin();
  delay(500);
}

void loop() {
  if (e220ttl.available()) {
		ResponseStructContainer rsc = e220ttl.receiveMessageRSSI(sizeof(Messaggione));
    switch(rsc.data[0]) { //QUI NON COMPILA PERCHÉ DATA É VOID POINTER!
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
        long f = Serial.parseInt();
        changeFrequency(f);
        break
      }
    }
  }
  delay(DELAY); //CI SERVE QUESTO DELAY?
}

void changeFrequency(long freq) {
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
      delay(250); //CI SERVE QUESTO DELAY?
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
