#include "LoRa_E220.h"

#define DEBUG 1

struct RocketData {
  float acc_lin[3];
  float acc_ang[3];
  float barometer;
};

unsigned long last = 0;
unsigned long waitTime = 200;

LoRa_E220 e220ttl(&Serial1, 3, 4, 6);  //  RX AUX M0 M1

bool backend_connected = false;
byte frequency = 0xFF;

void setup() {
  Serial.begin(9600);
  delay(2000);
  randomSeed(analogRead(0));
  // Sending Ready Signal to Backend
  e220ttl.begin();

  ResponseStructContainer c = e220ttl.getConfiguration();
	Configuration configuration = *(Configuration*) c.data;
	printParameters(configuration);

}

void loop() {
  if (Serial.available()) {
    char inChar;
    Serial.readBytes(&inChar, 1);
    switch (inChar) {
      case 'B':
        backend_connected = true;
        break;
      case 'W':
        waitTime = Serial.parseInt();
        Serial.print("Now delaying ");
        Serial.print(waitTime);
        Serial.println("ms");
        break;
      case 'F':
        {
          int f = Serial.parseInt();
          char msg[] = "F0";
          msg[1] = (char) f;
          Serial.print("mando frequenza ");
          Serial.print(f);
          Serial.print('\t');
          Serial.println(msg);
          ResponseStatus rs = e220ttl.sendMessage(msg);
          Serial.print("Frequency msg \t");
          Serial.println(rs.getResponseDescription());
          ResponseContainer rc;
          rc.data = "A";

          while (rc.data[0] != 'C'){ // wait until response
            if(e220ttl.available() > 1){
              Serial.println("Message received");   // LOG - TO BE ELIMINATED
              rc = e220ttl.receiveMessage();
              if (rc.status.code!=1){
                Serial.println(rc.status.getResponseDescription());
              }else{  // print received data
                Serial.println(rc.status.getResponseDescription());
                Serial.println(rc.data);
                ResponseStatus rs = e220ttl.sendMessage("A");
              }
            }
          }

          break;
        }
    }
  }
  
  if (e220ttl.available()) {
    Serial.print("Received: ");
    ResponseContainer rc = e220ttl.receiveMessage();

    if (rc.status.code != 1) { // errore
      Serial.print("errore -> ");
      Serial.println(rc.status.getResponseDescription());
    } else {
      Serial.print("raw = ");
      Serial.print(rc.data);
      unsigned long now = millis();
      unsigned long delta = now - last;
      Serial.print("\t {"); 
      if(delta >= 10000) {Serial.print(delta / 1000.0); Serial.println("s}");}
      else {Serial.print(delta); Serial.println("ms}");}
      last = now;
      switch(rc.data[0]) {
      case 'C':
        {
          Serial.print("Resend C: [");
          // ResponseStatus rs;
          // for(unsigned i=0; i<10; i++) {
          //   rs = e220ttl.sendMessage("C");
          //   Serial.print(i+1);
          //   Serial.print(' ');
          //   Serial.print(rs.getResponseDescription());
          //   Serial.print(", ");
          //   delay(250);
          // }
          Serial.println(']');
          break;
        }
      case 'D':
        {
          Serial.println("Dati ricevuti");
          break;
        }
      }
    }
  }
  delay(waitTime);
}

void printParameters(struct Configuration& configuration) {
	Serial.println("----------------------------------------");
	Serial.print(F("HEAD : "));  Serial.print(configuration.COMMAND, HEX);Serial.print(" ");Serial.print(configuration.STARTING_ADDRESS, HEX);Serial.print(" ");Serial.println(configuration.LENGHT, HEX);
	Serial.println(F(" "));
	Serial.print(F("AddH : "));  Serial.println(configuration.ADDH, HEX);
	Serial.print(F("AddL : "));  Serial.println(configuration.ADDL, HEX);
	Serial.println(F(" "));
	Serial.print(F("Chan : "));  Serial.print(configuration.CHAN, DEC); Serial.print(" -> "); Serial.println(configuration.getChannelDescription());
	Serial.println(F(" "));
	Serial.print(F("SpeedParityBit     : "));  Serial.print(configuration.SPED.uartParity, BIN);Serial.print(" -> "); Serial.println(configuration.SPED.getUARTParityDescription());
	Serial.print(F("SpeedUARTDatte     : "));  Serial.print(configuration.SPED.uartBaudRate, BIN);Serial.print(" -> "); Serial.println(configuration.SPED.getUARTBaudRateDescription());
	Serial.print(F("SpeedAirDataRate   : "));  Serial.print(configuration.SPED.airDataRate, BIN);Serial.print(" -> "); Serial.println(configuration.SPED.getAirDataRateDescription());
	Serial.println(F(" "));
	Serial.print(F("OptionSubPacketSett: "));  Serial.print(configuration.OPTION.subPacketSetting, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getSubPacketSetting());
	Serial.print(F("OptionTranPower    : "));  Serial.print(configuration.OPTION.transmissionPower, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getTransmissionPowerDescription());
	Serial.print(F("OptionRSSIAmbientNo: "));  Serial.print(configuration.OPTION.RSSIAmbientNoise, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getRSSIAmbientNoiseEnable());
	Serial.println(F(" "));
	Serial.print(F("TransModeWORPeriod : "));  Serial.print(configuration.TRANSMISSION_MODE.WORPeriod, BIN);Serial.print(" -> "); Serial.println(configuration.TRANSMISSION_MODE.getWORPeriodByParamsDescription());
	Serial.print(F("TransModeEnableLBT : "));  Serial.print(configuration.TRANSMISSION_MODE.enableLBT, BIN);Serial.print(" -> "); Serial.println(configuration.TRANSMISSION_MODE.getLBTEnableByteDescription());
	Serial.print(F("TransModeEnableRSSI: "));  Serial.print(configuration.TRANSMISSION_MODE.enableRSSI, BIN);Serial.print(" -> "); Serial.println(configuration.TRANSMISSION_MODE.getRSSIEnableByteDescription());
	Serial.print(F("TransModeFixedTrans: "));  Serial.print(configuration.TRANSMISSION_MODE.fixedTransmission, BIN);Serial.print(" -> "); Serial.println(configuration.TRANSMISSION_MODE.getFixedTransmissionDescription());
	Serial.println("----------------------------------------");
}
