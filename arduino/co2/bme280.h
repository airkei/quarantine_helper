#ifndef __bme280_h__
#define __bme280_h__

#include <SparkFunBME280.h>
#include <Wire.h>

void bme280_setup(int pin);
bool bme280_read(float *temp, float *hum, float *pressure);

#endif
