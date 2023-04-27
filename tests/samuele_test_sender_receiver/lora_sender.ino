#include "Arduino.h"
#include "LoRa_E220.h"

LoRa_E220 e220ttl(&Serial1, 2, 4, 6); //  RX AUX M0 M1


void setup() {
  Serial.begin(9600);
  e220ttl.begin();
  pinMode(8, OUTPUT);
  delay(500);  
}

int i = 0;

void loop() {
  String msg = ( ":)" );
  if ( i++ % 2 == 0 ) digitalWrite(8, LOW);
    else digitalWrite(8, HIGH);
  ResponseStatus rs = e220ttl.sendFixedMessage(0, 3, 40, msg);
  delay(2000);
}
