#include "LoRa_E220.h"

LoRa_E220 e220ttl(&Serial1, 3, 4, 6);  //  RX AUX M0 M1

bool backend_connected = false;
byte frequency = 0xFF;

void setup() {

  Serial.begin(9600);
  randomSeed(analogRead(0));
  // Sending Ready Signal to Backend
  e220ttl.begin();
  delay(500);
}

void loop() {
  // if (Serial.available()) {
  //   char inChar;
  //   Serial.readBytes(&inChar, 1);
  //   switch (inChar) {
  //     case 'B':
  //       backend_connected = true;
  //       break;
  //     case 'F':
  //       {
  //         Serial.readBytes(&inChar, 1);
  //         frequency = inChar;

  //         ResponseStructContainer c = e220ttl.getConfiguration();
  //         Configuration configuration = *((Configuration*)c.data);
  //         configuration.ADDH = 0x00;
  //         configutation.ADDL = 0x03;
  //         configuration.CHAN = 23;
  //         ResponseStatus rs = e220ttl.setConfiguration(configuration, WRITE_CFG_PWR_DWN_SAVE);
  //         c.close();
  //         // // emulating COM CHECK or no response
  //         // int randNumber = random(10);
  //         // if (randNumber > 5) {
  //         //   Serial.write('C');
  //         //   Serial.write('\0');
  //         //   ResponseStatus rs = e220ttl.sendMessage("Hello, world?");
  //         //   // Check If there is some problem of succesfully send
  //         // }
  //         break;
  //       }
  //   }
  // }
  // if (frequency == 0xFF) {
  //   if (!backend_connected) {
  //     Serial.write('R');
  //     Serial.write('\0');
  //     delay(500);
  //   }
  //   return;
  // }

  delay(350);
  
  if (e220ttl.available() > 1) {
    Serial.println("Message available");
    ResponseContainer rc = e220ttl.receiveMessage();

    if (rc.status.code != 1) {
      Serial.println(rc.status.getResponseDescription());
    } else {
      // Print the data received
      Serial.println(rc.status.getResponseDescription());
      Serial.println(rc.data);
    }

    //switch(rc.data){
        if (rc.data[0] == 'C')
        {
          Serial.println("C received, sending C");
          delay(450);
          ResponseStatus rs = e220ttl.sendFixedMessage(0, 2, 40, "C");
          Serial.println(rs.getResponseDescription());
        }

        if (rc.data[0] == 'A')
        {
          Serial.println("A received");
          delay(450);
        }
      //}
    /*
  // waiting for backend ready signal
  if (backend_connected && frequency != 0xFF) {

    int randNumber;
    // COM CHECK

    // Emulating errors
    randNumber = random(40);
    if (randNumber > 30) {
      Serial.write('E');
      Serial.write('1');
      return;
    }

    if (randNumber > 20) {
      Serial.write('E');
      Serial.write('2');
      return;
    }

    if (randNumber > 10) {
      Serial.write('E');
      Serial.write('3');
      return;
    }

    // ALL OK
    Serial.write('O');
    delay(5000);
  }
  */
  }
}
  void printParameters(struct Configuration configuration) {
    DEBUG_PRINTLN("----------------------------------------");

    DEBUG_PRINT(F("HEAD : "));
    DEBUG_PRINT(configuration.COMMAND, HEX);
    DEBUG_PRINT(" ");
    DEBUG_PRINT(configuration.STARTING_ADDRESS, HEX);
    DEBUG_PRINT(" ");
    DEBUG_PRINTLN(configuration.LENGHT, HEX);
    DEBUG_PRINTLN(F(" "));
    DEBUG_PRINT(F("AddH : "));
    DEBUG_PRINTLN(configuration.ADDH, HEX);
    DEBUG_PRINT(F("AddL : "));
    DEBUG_PRINTLN(configuration.ADDL, HEX);
    DEBUG_PRINTLN(F(" "));
    DEBUG_PRINT(F("Chan : "));
    DEBUG_PRINT(configuration.CHAN, DEC);
    DEBUG_PRINT(" -> ");
    DEBUG_PRINTLN(configuration.getChannelDescription());
    DEBUG_PRINTLN(F(" "));
    DEBUG_PRINT(F("SpeedParityBit     : "));
    DEBUG_PRINT(configuration.SPED.uartParity, BIN);
    DEBUG_PRINT(" -> ");
    DEBUG_PRINTLN(configuration.SPED.getUARTParityDescription());
    DEBUG_PRINT(F("SpeedUARTDatte     : "));
    DEBUG_PRINT(configuration.SPED.uartBaudRate, BIN);
    DEBUG_PRINT(" -> ");
    DEBUG_PRINTLN(configuration.SPED.getUARTBaudRateDescription());
    DEBUG_PRINT(F("SpeedAirDataRate   : "));
    DEBUG_PRINT(configuration.SPED.airDataRate, BIN);
    DEBUG_PRINT(" -> ");
    DEBUG_PRINTLN(configuration.SPED.getAirDataRateDescription());
    DEBUG_PRINTLN(F(" "));
    DEBUG_PRINT(F("OptionSubPacketSett: "));
    DEBUG_PRINT(configuration.OPTION.subPacketSetting, BIN);
    DEBUG_PRINT(" -> ");
    DEBUG_PRINTLN(configuration.OPTION.getSubPacketSetting());
    DEBUG_PRINT(F("OptionTranPower    : "));
    DEBUG_PRINT(configuration.OPTION.transmissionPower, BIN);
    DEBUG_PRINT(" -> ");
    DEBUG_PRINTLN(configuration.OPTION.getTransmissionPowerDescription());
    DEBUG_PRINT(F("OptionRSSIAmbientNo: "));
    DEBUG_PRINT(configuration.OPTION.RSSIAmbientNoise, BIN);
    DEBUG_PRINT(" -> ");
    DEBUG_PRINTLN(configuration.OPTION.getRSSIAmbientNoiseEnable());
    DEBUG_PRINTLN(F(" "));
    DEBUG_PRINT(F("TransModeWORPeriod : "));
    DEBUG_PRINT(configuration.TRANSMISSION_MODE.WORPeriod, BIN);
    DEBUG_PRINT(" -> ");
    DEBUG_PRINTLN(configuration.TRANSMISSION_MODE.getWORPeriodByParamsDescription());
    DEBUG_PRINT(F("TransModeEnableLBT : "));
    DEBUG_PRINT(configuration.TRANSMISSION_MODE.enableLBT, BIN);
    DEBUG_PRINT(" -> ");
    DEBUG_PRINTLN(configuration.TRANSMISSION_MODE.getLBTEnableByteDescription());
    DEBUG_PRINT(F("TransModeEnableRSSI: "));
    DEBUG_PRINT(configuration.TRANSMISSION_MODE.enableRSSI, BIN);
    DEBUG_PRINT(" -> ");
    DEBUG_PRINTLN(configuration.TRANSMISSION_MODE.getRSSIEnableByteDescription());
    DEBUG_PRINT(F("TransModeFixedTrans: "));
    DEBUG_PRINT(configuration.TRANSMISSION_MODE.fixedTransmission, BIN);
    DEBUG_PRINT(" -> ");
    DEBUG_PRINTLN(configuration.TRANSMISSION_MODE.getFixedTransmissionDescription());


    DEBUG_PRINTLN("----------------------------------------");
  }
