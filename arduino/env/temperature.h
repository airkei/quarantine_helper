#ifndef __temperature_h__
#define __temperature_h__

#include <Arduino_HTS221.h>

void temperature_setup();
bool temperature_read(float* temperature, float* humidity);

#endif
