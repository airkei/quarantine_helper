#include "co2_pas.h"

SensirionI2CScd4x scd4x;

void co2_pas_setup() {
    uint16_t error;
    char errorMessage[256];
    uint16_t serial0, serial1, serial2;

    // Initialize sensor
    scd4x.begin(Wire);
    // stop potentially previously started measurement
    error = scd4x.stopPeriodicMeasurement();
    if (error) {
        Serial.print("Error trying to execute stopPeriodicMeasurement(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    }

    error = scd4x.getSerialNumber(serial0, serial1, serial2);
    if (error) {
        Serial.print("Error trying to execute getSerialNumber(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    }

    // Start Measurement
    error = scd4x.startPeriodicMeasurement();
    if (error) {
        Serial.print("Error trying to execute startPeriodicMeasurement(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    }
}

bool co2_pas_read(uint16_t *co2, float *temperature, float *humidity) {
    uint16_t error;

    // Read Measurement
    error = scd4x.readMeasurement(*co2, *temperature, *humidity);
    if (error) {
        return false;
    }

    if (*co2 == 0) {
        return false;
    }
    return true;
}
