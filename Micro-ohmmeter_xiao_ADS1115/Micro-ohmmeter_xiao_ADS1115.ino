#include <Adafruit_BusIO_Register.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_I2CRegister.h>
#include <Adafruit_SPIDevice.h>

#include <Wire.h>
#include <Adafruit_ADS1X15.h>

Adafruit_ADS1115 ads;



int16_t results1;
int16_t results2;
float multiplier_mv = 0.0078125F;
float multiplier_v = 0.0000078125F;

float Rs = 0.2;

float Vs;
float Vdut;
float Is;
float Rdut;


long timeStart;
long timeStop;

void setup() {
  Serial.begin(2000000);
  Serial.println("Hello");

  ads.begin();

  Serial.println("diff read of AIN0 and AIN1");
  Serial.println("ADC Range: 16x gain  +/- 0.256V  1 bit = 0.0078125mV");

  ads.setGain(GAIN_SIXTEEN);

}

void loop() {
  timeStart = micros();
  results1 = ads.readADC_Differential_0_1();
  results2 = ads.readADC_Differential_2_3();
  timeStop = micros();
  Serial.print("time: ");
  Serial.println(timeStop - timeStart);

    Serial.print("Differential_1: "); Serial.print(results1); Serial.print("("); Serial.print((results1 * multiplier_mv),6); Serial.println("mV)");
    Serial.print("Differential_2: "); Serial.print(results2); Serial.print("("); Serial.print((results2 * multiplier_mv),6); Serial.println("mV)");

    Vs = results1*multiplier_v;
    Vdut = results2*multiplier_v;

    Is = Vs/Rs;
    Rdut = Vdut/Is;
    Serial.print("Is: "); Serial.println(Is,6);
    Serial.print("Rdut: "); Serial.println(Rdut,6);
    
}
