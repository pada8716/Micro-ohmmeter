#include <Adafruit_BusIO_Register.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_I2CRegister.h>
#include <Adafruit_SPIDevice.h>

#include <Wire.h>
#include <Adafruit_ADS1X15.h>

Adafruit_ADS1115 ads;

float V_DAC_v = 1;
float V_DACref_v = 3.3;
float pulse_on_us = 70000; //10% duty cycle
float pulse_period_us = 700000;
unsigned long last_us = micros(); //last time
long V_DAC_b = (V_DAC_v / V_DACref_v) * 1023; //convert DAC_v from V to bits

float Rs_ohm = 0.2;

int16_t results1on_b;
int16_t results2on_b;
int16_t results1off_b;
int16_t results2off_b;

float multiplier_mv = 0.0078125F;
float multiplier_v = 0.0000078125F;

float results1on_sum_b;
float results2on_sum_b;
float results1off_sum_b;
float results2off_sum_b;

float Vs_v;
float Vdut_v;
float Is_A;
float Rdut_ohm;

float Vs_sum_v;
float Vdut_sum_v;

int sum_counter = 0;
int iCounter_on = 0;
int iCounter_off = 0;
long iTimer_on_us;
long iTimer_off_us;

float averagingTime_us = 40000; //40ms -> 3 readings

long timeStart;
long timeStop;

void setup() {
  Serial.begin(2000000);
  Serial.println("Hello");

  ads.begin();

  Serial.println("diff read of AIN0 and AIN1");
  Serial.println("ADC Range: 16x gain  +/- 0.256V  1 bit = 0.0078125mV");

  ads.setGain(GAIN_SIXTEEN);

  analogWriteResolution(10); //10bit res analog out
  analogReadResolution(12);  //12bit res analog in

}

void loop() {

  while (micros() - last_us < pulse_on_us) {
    analogWrite(A0, V_DAC_b); //turn on
    sum_counter++;
    delay(5);

    iTimer_on_us = micros();

    while (micros() <= iTimer_on_us + averagingTime_us) {
      iCounter_on++;
      results1on_b = ads.readADC_Differential_0_1();
      results2on_b = ads.readADC_Differential_2_3();

      results1on_sum_b += results1on_b;
      results2on_sum_b += results2on_b;
      Serial.print("Diff1: "); Serial.println(results1on_b * multiplier_mv);
      Serial.print("Diff2: "); Serial.println(results2on_b* multiplier_mv);
    }

    while (micros() - last_us < pulse_on_us) {} // Stall until condition is met

  }

  //turn off if time is > pulse_on_us
  analogWrite(A0, 0);

  Serial.print("Differential_1: "); Serial.print((results1on_sum_b / (iCounter_on * 1.0) * multiplier_mv), 6); Serial.println("mV");
  Serial.print("Differential_2: "); Serial.print((results2on_sum_b / (iCounter_on * 1.0)* multiplier_mv), 6); Serial.println("mV");

  delay(5);

  iTimer_off_us = micros();

  //  timeStart = micros();
  while (micros() <= iTimer_off_us + averagingTime_us) {
    iCounter_off++;
    results1off_b = ads.readADC_Differential_0_1();
    results2off_b = ads.readADC_Differential_2_3();

    results1off_sum_b += results1off_b;
      results2off_sum_b += results2off_b;
  }

  //  timeStop = micros();
  //  Serial.print("time: "); Serial.println(timeStop - timeStart);

  Serial.print("Differential_1: "); Serial.print((results1off_sum_b / (iCounter_off * 1.0) * multiplier_mv), 6); Serial.println("mV");
  Serial.print("Differential_2: "); Serial.print((results2off_sum_b / (iCounter_off * 1.0) * multiplier_mv), 6); Serial.println("mV");

  Vs_v = (results1on_sum_b / (iCounter_on * 1.0) * multiplier_v) - (results1off_sum_b / (iCounter_off * 1.0) * multiplier_v);
  Vdut_v = (results2on_sum_b / (iCounter_on * 1.0) * multiplier_v) - (results2off_sum_b / (iCounter_off * 1.0) * multiplier_v);

  results1on_sum_b = 0; results1off_sum_b = 0; results2on_sum_b = 0; results2off_sum_b = 0;
  iCounter_on = 0; iCounter_off = 0;

  Vs_sum_v += Vs_v;
  Vdut_sum_v += Vdut_v;

  Is_A = (Vs_sum_v / (sum_counter * 1.0)) / Rs_ohm;
  Rdut_ohm = (Vdut_sum_v / (sum_counter * 1.0)) / Is_A;
  Serial.print("Is: "); Serial.print(Is_A, 6); Serial.println(" A");
  Serial.print("Rdut: "); Serial.print(Rdut_ohm, 6); Serial.println(" ohm");

  while (micros() - last_us <= pulse_period_us) {} //Stall until condition is met

  //reset increment if time = period
  if (micros() - last_us >= pulse_period_us) {
    //    Serial.print("period time: "); Serial.println((micros() - last_us) / 1000.0);
    last_us = micros();
  }

}
