#ifndef __common_h__
#define __common_h__

#include <Arduino.h>

#define SAMPLES 128  // Must be a power of 2

typedef enum {
    ACTION_NONE = 0,
    ACTION_GRAB,
    ACTION_HANDLE,
    ACTION_LOCK,
    ACTION_DOOR,
    ACTION_MAX
} ACTION;

#endif
