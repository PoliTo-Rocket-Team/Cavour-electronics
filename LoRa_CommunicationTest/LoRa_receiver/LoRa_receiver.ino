#include "Arduino.h"
#include "LoRa_E220.h"

// #define LoRa_E220_DEBUG 1

struct RocketData {
  float acc_lin[3];
  float acc_ang[3];
  float barometer;
};

unsigned waitTime = 100;


void changeFrequency(unsigned freq);
void sendData();
float randomFloat(float, float);

LoRa_E220 e220ttl(&Serial1, 2, 4, 6);  //  RX AUX M0 M1

void setup() {
  Serial.begin(9600);
  e220ttl.begin();

  randomSeed(analogRead(0));

  delay(5000);

  // ResponseStructContainer c;
  // c = e220ttl.getConfiguration();
  // Configuration configuration = *(Configuration*) c.data;
  // printParameters(configuration);

  /*
  ResponseContainer rc;
  rc.data = "A";
  String msg;
  while (rc.data[0] != 'C'){ // wait until GS response
    msg = "C";
    ResponseStatus rs = e220ttl.sendMessage(msg);
    Serial.print("Send start C \t");
    Serial.println(rs.getResponseDescription());
    delay(waitTime);

    if(e220ttl.available()){
      Serial.print("Message received -> ");   // LOG - TO BE ELIMINATED
      rc = e220ttl.receiveMessage();
      if (rc.status.code!=1){
        Serial.print("error: ");
        Serial.println(rc.status.getResponseDescription());
      }else{  // print received data
        Serial.print("raw: ");
        Serial.println(rc.data);
      }
    }
  }
  */
}

void loop() {
  // Serial.println("Loop");
  if (Serial.available()) {
    char inChar;
    Serial.readBytes(&inChar, 1);
    switch (inChar) {
      case 'W':
        waitTime = Serial.parseInt();
        Serial.print("Now delaying ");
        Serial.print(waitTime);
        Serial.println(" ms");
        break;
    }
  }

  if (e220ttl.available()) {
    Serial.print("Message received -> ");  // LOG - TO BE ELIMINATED
    ResponseContainer rc = e220ttl.receiveMessage();
    if (rc.status.code != 1) {
      Serial.print("error: ");
      Serial.println(rc.status.getResponseDescription());
    } else {  // print received data
      Serial.print("raw: ");
      Serial.println(rc.data);
    }
  } else sendData();
  delay(waitTime);
}

void changeFrequency(unsigned freq) {
  ResponseStructContainer c;
  c = e220ttl.getConfiguration();
  // It's important get configuration pointer before all other operation
  Configuration configuration = *(Configuration*)c.data;
  Serial.println(c.status.getResponseDescription());
  Serial.println(c.status.code);

  Serial.print("Mi hai detto che la frequenza e' \t");
  Serial.println(freq);

  // printParameters(configuration);

  ResponseContainer rc;
  rc.data = "D";

  do {
    ResponseStatus rs = e220ttl.sendMessage("C");
    Serial.println(rs.getResponseDescription());
    delay(250);
    if (e220ttl.available() == 0) continue;
    Serial.println("Message received");  // LOG - TO BE ELIMINATED
    rc = e220ttl.receiveMessage();
    if (rc.status.code != 1) {
      Serial.println(rc.status.getResponseDescription());
    } else {  // print received data
      Serial.println(rc.status.getResponseDescription());
      Serial.println(rc.data);
    }
  } while (rc.data[0] != 'A');
}

void sendData() {
  /*
  RocketData msg;
  msg.acc_lin[0] = randomFloat(-3, +3);
  msg.acc_lin[1] = randomFloat(-3, +3);
  msg.acc_lin[2] = randomFloat(-3, +3);
  msg.acc_ang[0] = randomFloat(-3, +3);
  msg.acc_ang[1] = randomFloat(-3, +3);
  msg.acc_ang[2] = randomFloat(-3, +3);
  msg.barometer = randomFloat(0, 1000);
  char str[2 + sizeof(RocketData)];
  str[0] = 'D';
  str[1 + sizeof(RocketData)] = '\0';
  memcpy(&str, (void*)&msg, sizeof(RocketData) + 1);
  Serial.print("Data sent (");
  for (unsigned i = 0; i < sizeof(RocketData); i++) {
    ResponseStatus rs = e220ttl.sendMessage(&str[i]);
    Serial.print(str[i + 1], DEC);
    Serial.print('|');
  }
*/
  struct Data {  // Partial structure without type
    char code;
    float bar;
    float al_x;
    float al_y;
    float al_z;
    float aa_x;
    float aa_y;
    float aa_z;
  };

    struct Data packet = {
    'D',
    // randomFloat(0, 1000),
    // randomFloat(-3, +3),
    // randomFloat(-3, +3),
    // randomFloat(-3, +3),
    // randomFloat(-3, +3),
    // randomFloat(-3, +3),
    // randomFloat(-3, +3),
    (float) 1,
    (float) 2,
    (float) 3,
    (float) 4,
    (float) 5,
    (float) 6,
    (float) 7,

  };
  ResponseStatus rs = e220ttl.sendMessage(&packet, sizeof(Data));

  // Serial.print(") -> ");
  // Serial.println(rs.getResponseDescription());
}

float randomFloat(float minf, float maxf) {
  return minf + random(1UL << 31) * (maxf - minf) / (1UL << 31);  // use 1ULL<<63 for max double values)
}