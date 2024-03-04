#include "imu.h"

void imu_setup() {
    if (!IMU.begin()) {
        Serial.println("Failed to initialize IMU!");
        while (1)
            ;
    }
}

void imu_read(SENSORS_DATA *sensors, int sensor_counter) {
    float x = 0, y = 0, z = 0;

    if (IMU.accelerationAvailable()) {
        IMU.readAcceleration(x, y, z);
        sensors->sensor[SENSOR_KIND_ACC].x[sensor_counter] = (double)x;
        sensors->sensor[SENSOR_KIND_ACC].y[sensor_counter] = (double)y;
        sensors->sensor[SENSOR_KIND_ACC].z[sensor_counter] = (double)z;
    }

    if (IMU.gyroscopeAvailable()) {
        IMU.readGyroscope(x, y, z);
        sensors->sensor[SENSOR_KIND_GYRO].x[sensor_counter] = (double)x;
        sensors->sensor[SENSOR_KIND_GYRO].y[sensor_counter] = (double)y;
        sensors->sensor[SENSOR_KIND_GYRO].z[sensor_counter] = (double)z;
    }

    if (IMU.magneticFieldAvailable()) {
        IMU.readMagneticField(x, y, z);
        sensors->sensor[SENSOR_KIND_MAG].x[sensor_counter] = (double)x;
        sensors->sensor[SENSOR_KIND_MAG].y[sensor_counter] = (double)y;
        sensors->sensor[SENSOR_KIND_MAG].z[sensor_counter] = (double)z;
    }
}
