#ifndef __imu_h__
#define __imu_h__

#include <Arduino_LSM9DS1.h>

#include "common.h"

typedef enum {
    SENSOR_KIND_ACC = 0,
    SENSOR_KIND_GYRO = 1,
    SENSOR_KIND_MAG = 2,
    SENSOR_KIND_MAX
} SENSOR_KIND;

typedef struct {
    double x[SAMPLES * 2];
    double y[SAMPLES * 2];
    double z[SAMPLES * 2];
} SENSOR_DATA __attribute__((packed));

typedef struct {
    SENSOR_DATA sensor[SENSOR_KIND_MAX];
} SENSORS_DATA;

void imu_setup();
void imu_read(SENSORS_DATA *sensors, int sensor_counter);

#endif
