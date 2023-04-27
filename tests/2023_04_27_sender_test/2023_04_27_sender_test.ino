#include "Arduino.h"
#include "LoRa_E220.h"

LoRa_E220 e220ttl(&Serial1, 2, 4, 6); //  RX AUX M0 M1

void setup() {
  Serial.begin(9600);
  delay(500);
  e220ttl.begin();

  Serial.println("Inizializzato. Inizio trasmissione.");

  ResponseContainer rc;

  while (rc.data != "C"){
    ResponseStatus rs = e220ttl.sendFixedMessage(0, 3, 23, "C");
    Serial.println(rs.getResponseDescription());
    delay(200);

    if(e220ttl.available() > 1){
      Serial.println("Message received porcodio");
      rc = e220ttl.receiveMessage();
      if (rc.status.code!=1){
        Serial.println(rc.status.getResponseDescription());
      }else{
      // Print the data received
        Serial.println(rc.status.getResponseDescription());
        Serial.println(rc.data);
      }
    }
  }
}

void loop() {
  ResponseStatus rs = e220ttl.sendFixedMessage(0, 3, 23, "C");
  Serial.println(rs.getResponseDescription());
  Serial.println("dio animale");
  delay(200);
}
