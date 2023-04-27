#include "Arduino.h"
#include "LoRa_E220.h"
#define ENABLE_RSSI true

LoRa_E220 e220ttl(&Serial1, 2, 4, 6); //  RX AUX M0 M1

void setup() {

  Serial.begin(9600);
  e220ttl.begin();
  pinMode(8, OUTPUT);
  delay(500);

}

void loop() {

  if (e220ttl.available() > 1) {

    #ifdef ENABLE_RSSI 
	    ResponseContainer rc = e220ttl.receiveMessageRSSI();
    #endif

    Serial.println(rc.status.getResponseDescription());    
    Serial.println(rc.data); 
    
    Serial.print("RSSI: "); 
    Serial.println(rc.rssi, DEC);
    Serial.println();

    digitalWrite(8, LOW);    

  } else {
    Serial.println("Waiting...");
    Serial.println();

    digitalWrite(8, HIGH);
  }

  delay(1000);

}
