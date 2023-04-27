#include "Arduino.h"
#include "LoRa_E220.h"

LoRa_E220 e220ttl(&Serial1, 2, 4, 6); //  RX AUX M0 M1

void setup() {
  Serial.begin(9600);
  e220ttl.begin();
  delay(500);  
}

int i = 0;

void loop() {

  String msg;
  msg = String(i);

  ResponseStatus rs = e220ttl.sendMessage(msg);
  Serial.print("Attempt n. ");
  Serial.print(msg);
  Serial.print("\t -> \t");
  Serial.println(rs.getResponseDescription());

  i += 1;
  
  delay(1000);
}
