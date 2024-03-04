#include "voc.h"

SGP40 vocSensor;  // create an object of the SGP40 class

void voc_setup() {
    // Initialize sensor
    if (vocSensor.begin() == false) {
        Serial.println(F("SGP40 not detected. Check connections. Freezing..."));
    }
}

bool voc_read(uint16_t *voc) {
    *voc = vocSensor.getVOCindex();
    return true;
}
