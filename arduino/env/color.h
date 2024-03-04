#ifndef __color_h__
#define __color_h__

#include <Arduino_APDS9960.h>

void color_setup();
bool color_read(int* r, int* g, int* b, int* a);

#endif
