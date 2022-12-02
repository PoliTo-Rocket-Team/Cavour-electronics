#include "Arduino.h"
#include "LoRa_E220.h"

#define DESTINATION_ADDL 2

LoRa_E220 e220ttl(&Serial1, 2, 3, 4); //  RX AUX M0 M1

void printParameters(struct Configuration configuration);

void setup() {
  pinMode(7, OUTPUT);
  Serial.begin(9600);
  delay(500);

  // Startup all pins and UART
  e220ttl.begin();

  ResponseStructContainer c;
  c = e220ttl.getConfiguration();
  // It's important get configuration pointer before all other operation
  Configuration configuration = *(Configuration*) c.data;
  Serial.println(c.status.getResponseDescription());
  Serial.println(c.status.code);

  printParameters(configuration);
  c.close();

  Serial.println("Hi, I'm going to send message!");
  
  // Send message
  ResponseStatus rs = e220ttl.sendFixedMessage(0, DESTINATION_ADDL, 23, "Hello, world?");
  // Check If there is some problem of succesfully send
  Serial.println(rs.getResponseDescription());


}

void loop() {
  // If something available
  if (e220ttl.available()>1) {
    // read the String message
#ifdef ENABLE_RSSI
  ResponseContainer rc = e220ttl.receiveMessageRSSI();
#else
  ResponseContainer rc = e220ttl.receiveMessage();
#endif
  // Is something goes wrong print error
  if (rc.status.code!=1){
    Serial.println(rc.status.getResponseDescription());
  }else{
    // Print the data received
    Serial.println(rc.status.getResponseDescription());
    Serial.println(rc.data);

    if (rc.data == "song\n") {
      tone(7, 950);
      delay(100);
      noTone(7); 
    }
#ifdef ENABLE_RSSI
    Serial.print("RSSI: "); Serial.println(rc.rssi, DEC);
#endif
  }
  }
  if (Serial.available()) {
    String input = Serial.readString();
    ResponseStatus rs = e220ttl.sendFixedMessage(0, DESTINATION_ADDL, 23, input);
    // Check If there is some problem of succesfully send
    Serial.println(rs.getResponseDescription());
  }
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
