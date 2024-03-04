#ifndef __voc_h__
#define __voc_h__

#include "SparkFun_SGP40_Arduino_Library.h"

void voc_setup();
bool voc_read(uint16_t *voc);

#endif
