#include "Arduino.h"
#include "LoRa_E220.h"
 
LoRa_E220 e220ttl(&Serial1, -1, 4, 6); // ARDUINO NANO PINOUT - NO AUX
 
void setup() {

  Serial.begin(9600);
  delay(500);

  // FACTORY RESET - MO & M1 @ HIGH

  pinMode(4, OUTPUT);
  pinMode(6, OUTPUT);

  digitalWrite(4, HIGH);
  digitalWrite(6, HIGH);

  delay(100);

  // WRITE DEFAULT REGISTER PARAMETERS：C0 00 00 62 00 17

  byte buf[] = { 0xC0, 0x00, 0x00, 0x3E, 0x00, 0x11 };
  Serial.write( (uint8_t *) buf, 6);
  
  delay(500);
    
  Serial.println("Factory Reset!");

}

void loop() {}
