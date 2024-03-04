#include "temperature.h"

void temperature_setup() {
    if (!HTS.begin()) {
        Serial.println("Failed to initialize humidity temperature sensor!");
    }
}

bool temperature_read(float* temperature, float* humidity) {
    *temperature = HTS.readTemperature();
    *humidity = HTS.readHumidity();

    return true;
}
