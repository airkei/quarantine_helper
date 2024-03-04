#ifndef __dust_h__
#define __dust_h__

#include <Arduino.h>

void dust_setup(int pin);
bool dust_read(int pin, unsigned long ts, float temperature, double *dust);

#endif
