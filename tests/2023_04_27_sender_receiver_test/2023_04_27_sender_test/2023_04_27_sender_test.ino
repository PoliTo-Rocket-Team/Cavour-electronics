#include "Arduino.h"
#include "LoRa_E220.h"

LoRa_E220 e220ttl(&Serial1, 2, 4, 6); //  RX AUX M0 M1

void setup() {
  Serial.begin(9600);
  e220ttl.begin();
  
  delay(500);
  Serial.println("Inizializzato. Inizio trasmissione.");  // LOG - TO BE ELIMINATED

  ResponseContainer rc;
  rc.data = "A";

  while (rc.data != "C"){ // wait until GS response
    ResponseStatus rs = e220ttl.sendMessage("C");
    Serial.println(rs.getResponseDescription());
    delay(400);

    if(e220ttl.available() > 1){
      Serial.println("Message received");   // LOG - TO BE ELIMINATED
      rc = e220ttl.receiveMessage();
      if (rc.status.code!=1){
        Serial.println(rc.status.getResponseDescription());
      }else{  // print received data
        Serial.println(rc.status.getResponseDescription());
        Serial.println(rc.data);
      }
    }
    delay(400);
  }
}

void loop() {
  ResponseStatus rs = e220ttl.sendMessage("C");
  Serial.println(rs.getResponseDescription());
  Serial.println("SONO NEL LOOP"); // LOG - TO BE ELIMINATED
  delay(400);
}
