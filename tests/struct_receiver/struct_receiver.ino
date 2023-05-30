#include "Arduino.h"
#include "LoRa_E220.h"

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

  if (e220ttl.available()  > 1 ) {

		ResponseStructContainer rsc = e220ttl.receiveMessageRSSI(sizeof(Messaggione));
		struct Messaggione messaggione = *(Messaggione*) rsc.data;

    Serial.println( *(float*) messaggione.bar);
    Serial.println( *(float*) messaggione.acc_lx);
    Serial.println( *(float*) messaggione.temp);
    Serial.println( *(float*) messaggione.acc_ly);
    Serial.println( *(float*) messaggione.acc_lz);
    Serial.println( *(float*) messaggione.acc_ax);
    Serial.println( *(float*) messaggione.acc_ay);
    Serial.println( *(float*) messaggione.acc_az);
    Serial.println();
    Serial.print("RSSI: "); Serial.println(rsc.rssi, DEC); Serial.println();

    Serial.println("*****FINE*****"); Serial.println();


    rsc.close();

	}
}
