#include <NRF52_MBED_TimerInterrupt.h>
#include <mbed.h>
#include <rtos.h>

#include "common.h"
#include "emg.h"
#include "fft.h"
#include "imu.h"
#ifdef __BLE_ENABLE__
#include "ble_peripheral.h"
#endif
#include "inference.h"

/*** Definition ***/
//#define __COLLECT_RAW_DATA__        true

#define SAMPLING_FREQUENCY 100  // Hz, must be less than 10000 due to ADC
#define EMG_CH 2
#define INF_DATA_SIZE \
    (SAMPLES * 4)  // SAMPLES/2 * 8(emg0, emg1, acc.x, acc.y, acc.z, gyro.x,
                   // gyro.y, gyro.z)

#define SENSOR_FLAG_TMR_AVAILABLE (1UL << 0)
#define SENSOR_FLAG_BLE_AVAILABLE (1UL << 1)

#define INF_FLAG_DATA_A (1UL << 0)
#define INF_FLAG_DATA_B (1UL << 1)

#define DBG_BUFFER_SIZE 256

#define PORT_EMG0 A0
#define PORT_EMG1 A1
#define PORT_BUZZER D12

/*** Name Space ***/
using namespace rtos;

/*** Structure ***/
typedef struct {
    byte ble_act;
} message_t;

/*** Variables ***/
int sensor_counter = 0;
double emg_data[EMG_CH][SAMPLES * 2];
SENSORS_DATA imu_data;
ACTION ble_act = ACTION_NONE;

int grp_counter = 0;
Thread sensor_thread(osPriorityRealtime, 4 * 1024);
Thread inference_a_thread(osPriorityAboveNormal, 8 * 1024);
Thread inference_b_thread(osPriorityAboveNormal, 8 * 1024);
Thread ble_thread(osPriorityNormal, 4 * 1024);
EventFlags sensor_flags, inf_flags, ble_flags;
MemoryPool<message_t, 16> mpool;
Queue<message_t, 16> queue;

char dbg_buf[DBG_BUFFER_SIZE];
char dbg_max = 0;

// Init NRF52 timer NRF_TIMER3
NRF52_MBED_Timer ITimer0(NRF_TIMER_3);

/*** Interrupt ***/
void timer_handler() { sensor_flags.set(SENSOR_FLAG_TMR_AVAILABLE); }

/*** Functions ***/
#ifdef __BLE_ENABLE__
static void transmit_ble(ACTION act) {
    // BLE Transmit
    if (act != ble_act) {
        sensor_flags.clear(SENSOR_FLAG_BLE_AVAILABLE);  // stop sensor

        ble_act = act;
        message_t *message = mpool.alloc();
        message->ble_act = (byte)ble_act;
        queue.put(message);
    }
}

static void ble_thread_cb() {
    while (true) {
        osEvent evt = queue.get();
        if (evt.status == osEventMessage) {
            message_t *message = (message_t *)evt.value.p;

            Serial.print("[BLE]Notify:");
            Serial.println(message->ble_act);
            ble_peripheral_notify(message->ble_act);
            mpool.free(message);

            sensor_counter = 0;
            sensor_flags.set(SENSOR_FLAG_BLE_AVAILABLE);  // restart sensor
        }
    }
}
#endif

static void combine_data(double *emg0, double *emg1, SENSORS_DATA *sensor,
                         int cnt, float *output, int output_size) {
    int i = 0;

    memset(output, 0x00, output_size);

    calc_fft(&(emg0[cnt]), SAMPLES);
    calc_fft(&(emg1[cnt]), SAMPLES);
    calc_fft(&(sensor->sensor[SENSOR_KIND_ACC].x[cnt]), SAMPLES);
    calc_fft(&(sensor->sensor[SENSOR_KIND_ACC].y[cnt]), SAMPLES);
    calc_fft(&(sensor->sensor[SENSOR_KIND_ACC].z[cnt]), SAMPLES);
    calc_fft(&(sensor->sensor[SENSOR_KIND_GYRO].x[cnt]), SAMPLES);
    calc_fft(&(sensor->sensor[SENSOR_KIND_GYRO].y[cnt]), SAMPLES);
    calc_fft(&(sensor->sensor[SENSOR_KIND_GYRO].z[cnt]), SAMPLES);

    for (i = 0; i < SAMPLES / 2; i++) {
        if (i < 10) {
            emg0[cnt + i] = 0;
            emg1[cnt + i] = 0;
            sensor->sensor[SENSOR_KIND_ACC].x[cnt + i] = 0;
            sensor->sensor[SENSOR_KIND_ACC].y[cnt + i] = 0;
            sensor->sensor[SENSOR_KIND_ACC].z[cnt + i] = 0;
            sensor->sensor[SENSOR_KIND_GYRO].x[cnt + i] = 0;
            sensor->sensor[SENSOR_KIND_GYRO].y[cnt + i] = 0;
            sensor->sensor[SENSOR_KIND_GYRO].z[cnt + i] = 0;
        }

#ifdef __COLLECT_RAW_DATA__
        // print sensor values
        sprintf(dbg_buf,
                "[DATA]grp:%d, cnt:%d, sample:%d, emg0:%f, emg1:%f, acc_x:%f, "
                "acc_y:%f, acc_z:%f, gyro_x:%f, gyro_y:%f, gyro_z:%f",
                grp_counter, i, SAMPLES / 2, emg_data[0][cnt + i],
                emg_data[1][cnt + i],
                imu_data.sensor[SENSOR_KIND_ACC].x[cnt + i],
                imu_data.sensor[SENSOR_KIND_ACC].y[cnt + i],
                imu_data.sensor[SENSOR_KIND_ACC].z[cnt + i],
                imu_data.sensor[SENSOR_KIND_GYRO].x[cnt + i],
                imu_data.sensor[SENSOR_KIND_GYRO].y[cnt + i],
                imu_data.sensor[SENSOR_KIND_GYRO].z[cnt + i]);
        Serial.println(dbg_buf);
#else
        output[8 * i] = float(emg0[cnt + i]);
        output[8 * i + 1] = float(emg1[cnt + i]);
        output[8 * i + 2] = float(sensor->sensor[SENSOR_KIND_ACC].x[cnt + i]);
        output[8 * i + 3] = float(sensor->sensor[SENSOR_KIND_ACC].y[cnt + i]);
        output[8 * i + 4] = float(sensor->sensor[SENSOR_KIND_ACC].z[cnt + i]);
        output[8 * i + 5] = float(sensor->sensor[SENSOR_KIND_GYRO].x[cnt + i]);
        output[8 * i + 6] = float(sensor->sensor[SENSOR_KIND_GYRO].y[cnt + i]);
        output[8 * i + 7] = float(sensor->sensor[SENSOR_KIND_GYRO].z[cnt + i]);
#endif
    }

    grp_counter++;
}

static void inference_a_thread_cb() {
    while (true) {
        float data[INF_DATA_SIZE];
        ACTION act = ACTION_NONE;
        float prob = 0;

        inf_flags.wait_all(INF_FLAG_DATA_A, osWaitForever, false);

        // Inference
        combine_data(emg_data[0], emg_data[1], &imu_data, 0, data,
                     sizeof(data));
#ifndef __COLLECT_RAW_DATA__
        inference_exec(data, sizeof(data), &act, &prob);
#ifdef __BLE_ENABLE__
        transmit_ble(act);
#endif
        Serial.print("[BLE]Notify:");
        Serial.print(act);
        Serial.print("(");
        Serial.print(prob * 100);
        Serial.println("%)");
#endif
        // clear flag
        inf_flags.clear(INF_FLAG_DATA_A);
    }
}

static void inference_b_thread_cb() {
    while (true) {
        float data[INF_DATA_SIZE];
        ACTION act = ACTION_NONE;
        float prob = 0;

        inf_flags.wait_all(INF_FLAG_DATA_B, osWaitForever, false);

        // Inference
        combine_data(emg_data[0], emg_data[1], &imu_data, SAMPLES, data,
                     sizeof(data));
#ifndef __COLLECT_RAW_DATA__
        inference_exec(data, sizeof(data), &act, &prob);
#ifdef __BLE_ENABLE__
        transmit_ble(act);
#endif
        Serial.print("[BLE]Notify:");
        Serial.print(act);
        Serial.print("(");
        Serial.print(prob * 100);
        Serial.println("%)");
#endif
        // clear flag
        inf_flags.clear(INF_FLAG_DATA_B);
    }
}

static void sensor_thread_cb() {
    while (true) {
        sensor_flags.wait_all(
            SENSOR_FLAG_TMR_AVAILABLE | SENSOR_FLAG_BLE_AVAILABLE,
            osWaitForever, false);
        if ((sensor_counter == 0) || (sensor_counter == SAMPLES)) {
            //      Serial.println("[SENSOR]Scan start");
        }

        // EMG Sampling(100HZ)
        emg_data[0][sensor_counter] = emg_read(PORT_EMG0);
        emg_data[1][sensor_counter] = emg_read(PORT_EMG1);

        // IMU Sampling(100HZ)
        imu_read(&imu_data, sensor_counter);

        // beep
#ifdef __COLLECT_RAW_DATA__
        if (sensor_counter <= SAMPLES / 16) {
            digitalWrite(PORT_BUZZER, HIGH);
        } else {
            digitalWrite(PORT_BUZZER, LOW);
        }
#endif

        // call inference
        sensor_counter++;
        if (sensor_counter == SAMPLES) {
            //      Serial.println("[SENSOR]Scan a done");
            inf_flags.set(INF_FLAG_DATA_A);
        } else if (sensor_counter == SAMPLES * 2) {
            //      Serial.println("[SENSOR]Scan b done");
            inf_flags.set(INF_FLAG_DATA_B);
            sensor_counter = 0;
        }
        sensor_flags.clear(SENSOR_FLAG_TMR_AVAILABLE);
    }
}

void setup() {
    unsigned int sampling_period_us = 0;

    digitalWrite(LED_PWR, LOW);

    // initialize serial
    Serial.begin(115200);
    while (!Serial)
        ;

    // set gloval variables
    sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQUENCY));
    Serial.print("Sampling Period:");
    Serial.println(sampling_period_us);

    // setup each device
    emg_setup();
    imu_setup();

#ifndef __COLLECT_RAW_DATA__
#ifdef __BLE_ENABLE__
    ble_peripheral_setup();
#endif
    inference_setup();
#endif

#ifndef __COLLECT_RAW_DATA__
#ifdef __BLE_ENABLE__
    // create ble thread
    ble_thread.start(ble_thread_cb);
#endif
#endif

    // create sensor and inference thread
    sensor_thread.start(sensor_thread_cb);
    inference_a_thread.start(inference_a_thread_cb);
    inference_b_thread.start(inference_b_thread_cb);

    // set sensor flag
    sensor_flags.clear(SENSOR_FLAG_TMR_AVAILABLE);
    sensor_flags.set(SENSOR_FLAG_BLE_AVAILABLE);

    // start timer
    if (!(ITimer0.attachInterruptInterval(sampling_period_us, timer_handler))) {
        Serial.println(F("Can't set ITimer0. Select another freq. or timer"));
    }
}

void loop() {
#ifndef __COLLECT_RAW_DATA__
#ifdef __BLE_ENABLE__
    ble_peripheral_loop();
#endif
#endif
}
