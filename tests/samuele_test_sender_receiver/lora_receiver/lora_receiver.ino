#include "Arduino.h"
#include "LoRa_E220.h"

LoRa_E220 e220ttl(&Serial1, 2, 4, 6); //  RX AUX M0 M1

void setup() {

  Serial.begin(9600);
  e220ttl.begin();
  delay(500);

}

void loop() {

  if (e220ttl.available() > 1) {

    ResponseContainer rc = e220ttl.receiveMessage();

    Serial.println(rc.status.getResponseDescription());    
    Serial.println(rc.data); 
    
  } else {
    Serial.println("Waiting...");
    Serial.println();
  }

  delay(350);

}
