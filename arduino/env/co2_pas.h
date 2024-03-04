#ifndef __co2_pas_h__
#define __co2_pas_h__

#include <SensirionI2CScd4x.h>

void co2_pas_setup();
bool co2_pas_read(uint16_t *co2, float *temperature, float *humidity);

#endif
