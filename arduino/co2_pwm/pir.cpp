#include "pir.h"

void pir_setup(int pin) { pinMode(pin, INPUT); }

bool pir_read(int pin, char *pir) {
    int sensorValue = digitalRead(pin);

    *pir = (char)sensorValue;

    return true;
}
