/*----- LIBRARIES -----*/
#include <cmath>
#include <Arduino_LSM9DS1.h>  // libreria per sensore IMU

/*----- GLOBAL VARIABLES ----- */
  /* UTILITIES AND FLAGS */
  int count = 0;            // if count == VALUE_TBD then is_falling = true
  bool is_falling = false;  

/*---------------------*/
/*----- FUNCTIONS -----*/

void setup() {
/*----- GENERAL SETUP -----*/
  delay(2000);  // wait for serial door connection
  Serial.begin(9600); // open serial door, set framerate @ 9600 bps
  while (!Serial);
  Serial.println("Started");

/*----- IMU SETUP -----*/
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

}

void loop() {

/*----- VARIABLES -----*/
  /* IMU */
  float ax, ay, az; // accelerometer
  float gx, gy, gz; // gyroscope

/*----- COLLECT IMU DATA -----*/
  if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
    IMU.readAcceleration(ax, ay, az);
    IMU.readGyroscope(gx, gy, gz);
    
    Serial.print("ax:");  Serial.print(ax); Serial.print("\t");
    Serial.print("ay:");  Serial.print(ay); Serial.print("\t");
    Serial.print("az:");  Serial.print(az); Serial.print("\t\t");
    Serial.print("gx:");  Serial.print(gx); Serial.print("\t");
    Serial.print("gy:");  Serial.print(gy); Serial.print("\t");
    Serial.print("gz:");  Serial.println(gz);
  }

  if(az < 0){
    count ++;
    if(count == 100){
      is_falling = true;
    }
  }
  else
    count = 0;

  if(is_falling){
    Serial.println("RILASCIO PARACADUTE");
    while(1);
  }

}