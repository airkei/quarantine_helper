#include "emg.h"

void emg_setup() {}

double emg_read(int port) {
    int sensorValue = 0;
    float voltage = 0;

    sensorValue = analogRead(port);
    voltage = ((double)sensorValue) * 5 / 1023;

    return voltage;
}
