#ifndef __pir_h__
#define __pir_h__

#include <Arduino.h>

void pir_setup(int pin);
bool pir_read(int pin, char *pir);

#endif
