#include "Arduino.h"
#include "LoRa_E220.h"

// #define LoRa_E220_DEBUG 1

struct RocketData {
  float acc_lin[3];
  float acc_ang[3];
  float barometer;
};

unsigned waitTime = 2000;


void changeFrequency(unsigned freq);
void sendData();

LoRa_E220 e220ttl(&Serial1, 3, 4, 6); //  RX AUX M0 M1

void setup() {
  Serial.begin(9600);
  e220ttl.begin();

  delay(5000);
  
  ResponseStructContainer c;
	c = e220ttl.getConfiguration();
	Configuration configuration = *(Configuration*) c.data;
	printParameters(configuration);

  Serial.println("Inizializzato. Inizio trasmissione.");  // LOG - TO BE ELIMINATED
  
  ResponseContainer rc;
  rc.data = "A";
  String msg;
  unsigned long count = 0;
  while (rc.data[0] != 'C'){ // wait until GS response

    if(Serial.available()){
      char inChar;
      Serial.readBytes(&inChar, 1);
      switch (inChar){
        case 'W':
          waitTime = Serial.parseInt();
          count = 0;
          Serial.print("Now delaying "); Serial.print(waitTime); Serial.println(" ms");
          break;
      }
    }

    msg = "count pino ";
    msg += count++;
    ResponseStatus rs = e220ttl.sendMessage(msg);
    Serial.print("Send start C \t");
    Serial.println(rs.getResponseDescription());
    delay(waitTime);

    if(e220ttl.available()){
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

void printModuleInformation(struct ModuleInformation moduleInformation) {
	Serial.println("----------------------------------------");
	DEBUG_PRINT(F("HEAD: "));  DEBUG_PRINT(moduleInformation.COMMAND, HEX);DEBUG_PRINT(" ");DEBUG_PRINT(moduleInformation.STARTING_ADDRESS, HEX);DEBUG_PRINT(" ");DEBUG_PRINTLN(moduleInformation.LENGHT, DEC);

	Serial.print(F("Model no.: "));  Serial.println(moduleInformation.model, HEX);
	Serial.print(F("Version  : "));  Serial.println(moduleInformation.version, HEX);
	Serial.print(F("Features : "));  Serial.println(moduleInformation.features, HEX);
	Serial.println("----------------------------------------");

}
