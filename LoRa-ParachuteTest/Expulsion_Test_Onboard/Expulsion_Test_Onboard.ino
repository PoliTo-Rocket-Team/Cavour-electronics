#include "Arduino.h"
#include "LoRa_E220.h"

//pins
#define BUZZER 7
#define DROGUE A2
#define MAIN A3
//LoRa destiation address
#define DESTINATION_ADDL 2

LoRa_E220 e220ttl(&Serial1, 2, 3, 4); //  RX AUX M0 M1

void setup() {
  //set pins
  pinMode(BUZZER,OUTPUT);
  pinMode(DROGUE, OUTPUT);
  pinMode(MAIN, OUTPUT);

  //initialize LoRa
  e220ttl.begin();
  //set fixed receiver configuration
  ResponseStructContainer c = e220ttl.getConfiguration();
  Configuration configuration = *(Configuration*) c.data;
  configuration.ADDL = 0x03;
  configuration.ADDH = 0x00;
  
  configuration.CHAN = 23;
 
  configuration.SPED.uartBaudRate = UART_BPS_9600;
  configuration.SPED.airDataRate = AIR_DATA_RATE_010_24;
  configuration.SPED.uartParity = MODE_00_8N1;

  configuration.OPTION.subPacketSetting = SPS_200_00;
  configuration.OPTION.RSSIAmbientNoise = RSSI_AMBIENT_NOISE_DISABLED;
  configuration.OPTION.transmissionPower = POWER_22;

  configuration.TRANSMISSION_MODE.enableRSSI = RSSI_DISABLED;
  configuration.TRANSMISSION_MODE.fixedTransmission = FT_FIXED_TRANSMISSION;
  configuration.TRANSMISSION_MODE.enableLBT = LBT_DISABLED;
  configuration.TRANSMISSION_MODE.WORPeriod = WOR_2000_011;
  ResponseStatus rs0 = e220ttl.setConfiguration(configuration, WRITE_CFG_PWR_DWN_SAVE);
  
  //send test message
  ResponseStatus rs = e220ttl.sendFixedMessage(0, DESTINATION_ADDL, 23, "Hello, world?");
}

void loop() {
  if (e220ttl.available()>1) { //check if there is a received message
    ResponseContainer rc = e220ttl.receiveMessage();
    if (rc.status.code!=1) { //if error occurs transmit the status back
      ResponseStatus rs1 = e220ttl.sendFixedMessage(0, DESTINATION_ADDL, 23, rc.status.getResponseDescription());
    }else{
      if (rc.data == "fire\n") { //if the message received is the command for deployment:
      ResponseStatus rs2 = e220ttl.sendFixedMessage(0, DESTINATION_ADDL, 23, "Received. Starting countdown.\n");
      delay(1000);
      buzzer_countdown();
      analogWrite(DROGUE,255);
      delay(200);
      digitalWrite(DROGUE,LOW);
      ResponseStatus rs3 = e220ttl.sendFixedMessage(0, DESTINATION_ADDL, 23, "Success.\n");
      } else if (rc.data == "ready\n") { //test command to check communication
          ResponseStatus rs4 = e220ttl.sendFixedMessage(0, DESTINATION_ADDL, 23, "Ready for deployment\n.");
      }
    }
  }
}

void buzzer_countdown(){
    tone(BUZZER, 880);
    delay(500);
    noTone(BUZZER);
    delay(1000);
    tone(BUZZER, 880);
    delay(500);
    noTone(BUZZER);
    delay(1000);
    tone(BUZZER, 880);
    delay(500);
    noTone(BUZZER);
    delay(1000);
    tone(BUZZER, 880);
    delay(1000);
    noTone(BUZZER);
}
