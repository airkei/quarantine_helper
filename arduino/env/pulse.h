#ifndef __pulse_h__
#define __pulse_h__

#include <Arduino.h>

unsigned long newPulseIn(const byte pin, const byte state,
                         const unsigned long timeout = 1000000L);

#endif
