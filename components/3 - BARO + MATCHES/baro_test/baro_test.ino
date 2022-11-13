/*----- LIBRARIES -----*/
  #include <Wire.h>
  #include <SPI.h>
  #include <Adafruit_BMP280.h>

/*----- GLOBAL VARIABLES ----- */
  /*BMP*/
  #define BMP280_ADDRESS 0x76 // da i2c_scanner, indirizzo sicuro
  Adafruit_BMP280 bmp; // I2C

/*---------------------*/
/*----- FUNCTIONS -----*/

void setup() {
/*----- GENERAL SETUP -----*/
  delay(2000);  // wait for serial door connection
  Serial.begin(9600); // open serial door, set framerate @ 9600 bps
  while (!Serial);
  Serial.println("Started");

/*----- BMP SETUP -----*/
  unsigned status;
  status = bmp.begin(BMP280_ADDRESS);
  if (!status) {
    Serial.println("Failed to initialize bmp!");
    while (1);
  }

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
}

void loop() {
/*----- VARIABLES -----*/
  /* BMP */
  float temp, p, h;

/*----- COLLECT BMP DATA -----*/
  temp = bmp.readTemperature();
  p = bmp.readPressure();
  h = bmp.readAltitude(1013.25);

  Serial.print("Temp [*C]:");  Serial.print(temp); Serial.print("\t");
  Serial.print("p [Pa]:");  Serial.print(p); Serial.print("\t");
  Serial.print("h [m]:");  Serial.println(h);

  delay(200);

}
