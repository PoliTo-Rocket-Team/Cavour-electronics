#include "Arduino.h"
#include "LoRa_E220.h"

#define DEBUG 1

struct RocketData {
  float acc_lin[3];
  float acc_ang[3];
  float barometer;
};

void changeFrequency(unsigned freq);
void sendData();

LoRa_E220 e220ttl(&Serial1, 2, 4, 6); //  RX AUX M0 M1

void setup() {
  Serial.begin(9600);
  e220ttl.begin();
  
  delay(500);
  Serial.println("Inizializzato. Inizio trasmissione.");  // LOG - TO BE ELIMINATED
  
  ResponseContainer rc;
  rc.data = "A";

  while (rc.data[0] != 'C'){ // wait until GS response
    ResponseStatus rs = e220ttl.sendMessage("C");
    Serial.print("Send start C \t");
    Serial.println(rs.getResponseDescription());
    delay(450);

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
  }

}

void loop() {
  delay(400);
  Serial.println("Sono nel loop");

  if(e220ttl.available() > 1){
    Serial.println("Message received");   // LOG - TO BE ELIMINATED
    ResponseContainer rc = e220ttl.receiveMessage();
    if (rc.status.code!=1){
      Serial.println(rc.status.getResponseDescription());
    }else{  // print received data
      Serial.println(rc.status.getResponseDescription());
      Serial.println(rc.data);
    }
    if(rc.data[0] == 'F') changeFrequency(rc.data[1]);
  }
  else sendData();
}

void changeFrequency(unsigned freq) {
  ResponseStructContainer c;
  c = e220ttl.getConfiguration();
  // It's important get configuration pointer before all other operation
  Configuration configuration = *(Configuration*) c.data;
  Serial.println(c.status.getResponseDescription());
  Serial.println(c.status.code);

  Serial.print("Mi hai detto che la frequenza e' \t");
  Serial.println(freq);

  printParameters(configuration);

  ResponseContainer rc;
  rc.data = "D";

  do {
    ResponseStatus rs = e220ttl.sendMessage("C");
    Serial.println(rs.getResponseDescription());
    delay(250);
    if(e220ttl.available() == 0) continue;
    Serial.println("Message received");   // LOG - TO BE ELIMINATED
    rc = e220ttl.receiveMessage();
    if (rc.status.code!=1){
      Serial.println(rc.status.getResponseDescription());
    }else{  // print received data
      Serial.println(rc.status.getResponseDescription());
      Serial.println(rc.data);
    }
  } while(rc.data[0] != 'A');
}

void sendData() {
  RocketData msg;
  msg.acc_lin[0] = msg.acc_lin[1] = msg.acc_lin[2] = 2.3;
  msg.acc_ang[0] = msg.acc_ang[1] = msg.acc_ang[2] = 7.9;
  msg.barometer = 3.4;
  char str[2 + sizeof(RocketData)];
  str[0] = 'D';
  str[1 + sizeof(RocketData)] = '\0';
  strncpy((char*) &msg, str, sizeof(RocketData));
  ResponseStatus rs = e220ttl.sendMessage(str);
  Serial.print("Data sent (");
  Serial.print(str);
  Serial.print(')');
  Serial.print(" > ");
  Serial.println(rs.getResponseDescription());
}

void printParameters(struct Configuration configuration) {
	DEBUG_PRINTLN("----------------------------------------");

	DEBUG_PRINT(F("HEAD : "));  DEBUG_PRINT(configuration.COMMAND, HEX);DEBUG_PRINT(" ");DEBUG_PRINT(configuration.STARTING_ADDRESS, HEX);DEBUG_PRINT(" ");DEBUG_PRINTLN(configuration.LENGHT, HEX);
	DEBUG_PRINTLN(F(" "));
	DEBUG_PRINT(F("AddH : "));  DEBUG_PRINTLN(configuration.ADDH, HEX);
	DEBUG_PRINT(F("AddL : "));  DEBUG_PRINTLN(configuration.ADDL, HEX);
	DEBUG_PRINTLN(F(" "));
	DEBUG_PRINT(F("Chan : "));  DEBUG_PRINT(configuration.CHAN, DEC); DEBUG_PRINT(" -> "); DEBUG_PRINTLN(configuration.getChannelDescription());
	DEBUG_PRINTLN(F(" "));
	DEBUG_PRINT(F("SpeedParityBit     : "));  DEBUG_PRINT(configuration.SPED.uartParity, BIN);DEBUG_PRINT(" -> "); DEBUG_PRINTLN(configuration.SPED.getUARTParityDescription());
	DEBUG_PRINT(F("SpeedUARTDatte     : "));  DEBUG_PRINT(configuration.SPED.uartBaudRate, BIN);DEBUG_PRINT(" -> "); DEBUG_PRINTLN(configuration.SPED.getUARTBaudRateDescription());
	DEBUG_PRINT(F("SpeedAirDataRate   : "));  DEBUG_PRINT(configuration.SPED.airDataRate, BIN);DEBUG_PRINT(" -> "); DEBUG_PRINTLN(configuration.SPED.getAirDataRateDescription());
	DEBUG_PRINTLN(F(" "));
	DEBUG_PRINT(F("OptionSubPacketSett: "));  DEBUG_PRINT(configuration.OPTION.subPacketSetting, BIN);DEBUG_PRINT(" -> "); DEBUG_PRINTLN(configuration.OPTION.getSubPacketSetting());
	DEBUG_PRINT(F("OptionTranPower    : "));  DEBUG_PRINT(configuration.OPTION.transmissionPower, BIN);DEBUG_PRINT(" -> "); DEBUG_PRINTLN(configuration.OPTION.getTransmissionPowerDescription());
	DEBUG_PRINT(F("OptionRSSIAmbientNo: "));  DEBUG_PRINT(configuration.OPTION.RSSIAmbientNoise, BIN);DEBUG_PRINT(" -> "); DEBUG_PRINTLN(configuration.OPTION.getRSSIAmbientNoiseEnable());
	DEBUG_PRINTLN(F(" "));
	DEBUG_PRINT(F("TransModeWORPeriod : "));  DEBUG_PRINT(configuration.TRANSMISSION_MODE.WORPeriod, BIN);DEBUG_PRINT(" -> "); DEBUG_PRINTLN(configuration.TRANSMISSION_MODE.getWORPeriodByParamsDescription());
	DEBUG_PRINT(F("TransModeEnableLBT : "));  DEBUG_PRINT(configuration.TRANSMISSION_MODE.enableLBT, BIN);DEBUG_PRINT(" -> "); DEBUG_PRINTLN(configuration.TRANSMISSION_MODE.getLBTEnableByteDescription());
	DEBUG_PRINT(F("TransModeEnableRSSI: "));  DEBUG_PRINT(configuration.TRANSMISSION_MODE.enableRSSI, BIN);DEBUG_PRINT(" -> "); DEBUG_PRINTLN(configuration.TRANSMISSION_MODE.getRSSIEnableByteDescription());
	DEBUG_PRINT(F("TransModeFixedTrans: "));  DEBUG_PRINT(configuration.TRANSMISSION_MODE.fixedTransmission, BIN);DEBUG_PRINT(" -> "); DEBUG_PRINTLN(configuration.TRANSMISSION_MODE.getFixedTransmissionDescription());


	DEBUG_PRINTLN("----------------------------------------");
}
void printModuleInformation(struct ModuleInformation moduleInformation) {
	Serial.println("----------------------------------------");
	DEBUG_PRINT(F("HEAD: "));  DEBUG_PRINT(moduleInformation.COMMAND, HEX);DEBUG_PRINT(" ");DEBUG_PRINT(moduleInformation.STARTING_ADDRESS, HEX);DEBUG_PRINT(" ");DEBUG_PRINTLN(moduleInformation.LENGHT, DEC);

	Serial.print(F("Model no.: "));  Serial.println(moduleInformation.model, HEX);
	Serial.print(F("Version  : "));  Serial.println(moduleInformation.version, HEX);
	Serial.print(F("Features : "));  Serial.println(moduleInformation.features, HEX);
	Serial.println("----------------------------------------");

}
