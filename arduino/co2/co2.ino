#include <NRF52_MBED_TimerInterrupt.h>
#include <Wire.h>
#include <mbed.h>
#include <rtos.h>

#include "ble_peripheral.h"
#include "bme280.h"
#include "co2_ndir.h"
#include "color.h"
#include "pir.h"

#define BME280_CS_PIN D10
#define PIR_PIN D8

#define SAMPLING_PERIOD_US (1 * 1000 * 1000)
#define SAMPLING_TIMES 10

#define FLAG_TMR_AVAILABLE (1UL << 0)

/*** Name Space ***/
using namespace rtos;

/*** Structure ***/

/*** Variables ***/
int sampling_cnt = 0;
Thread sensor_thread;
EventFlags tmr_flags;

// Init NRF52 timer NRF_TIMER3
NRF52_MBED_Timer ITimer0(NRF_TIMER_3);

/*** Interrupt ***/
void timer_handler() { tmr_flags.set(FLAG_TMR_AVAILABLE); }

/*** Functions ***/
static void sensor_thread_cb() {
    osThreadSetPriority(osThreadGetId(), osPriorityAboveNormal);

    while (true) {
        tmr_flags.wait_all(FLAG_TMR_AVAILABLE, osWaitForever, false);

        int color_r = 0, color_g = 0, color_b = 0, color_a = 0;
        static int prev_color_r = 0, prev_color_g = 0, prev_color_b = 0,
                   prev_color_a = 0;
        uint16_t co2_ndir = 0;
        static uint16_t prev_co2_ndir = 0;
        float temp = 0, hum = 0, pressure = 0;
        char pir = 0;
        static unsigned char pir_cnt = 0;

        sampling_cnt++;
        if (sampling_cnt < SAMPLING_TIMES) {
            // co2(ndir)
            if ((sampling_cnt % 2) == 0) {
                co2_ndir_read(&prev_co2_ndir);
            }

            // pir
            pir_read(PIR_PIN, &pir);
            if (pir != 0) {
                pir_cnt++;
            }

            tmr_flags.clear(FLAG_TMR_AVAILABLE);
            continue;
        } else {
            sampling_cnt = 0;
        }

        // Color
        if (color_read(&color_r, &color_g, &color_b, &color_a) == true) {
            prev_color_r = color_r;
            prev_color_g = color_g;
            prev_color_b = color_b;
            prev_color_a = color_a;
            ble_peripheral_notify_color(color_a);
        } else {
            color_r = prev_color_r;
            color_g = prev_color_g;
            color_b = prev_color_b;
            color_a = prev_color_a;
        }
        Serial.print("color_a:");
        Serial.print(color_a);

        // BME280
        bme280_read(&temp, &hum, &pressure);
        ble_peripheral_notify_temperature(temp);
        ble_peripheral_notify_humidity(hum);
        ble_peripheral_notify_temperature(hum);
        ble_peripheral_notify_pressure(pressure);
        Serial.print(", temp:");
        Serial.print(temp);
        Serial.print(", hum:");
        Serial.print(hum);
        Serial.print(", pressure:");
        Serial.print(pressure);

        // CO2(NDIR)
        if (co2_ndir_read(&co2_ndir) == true) {
            ble_peripheral_notify_co2_ndir(co2_ndir);
        } else {
            co2_ndir = prev_co2_ndir;
        }
        ble_peripheral_notify_co2_ndir(co2_ndir);
        Serial.print(", co2:");
        Serial.print(co2_ndir);
        prev_co2_ndir = co2_ndir;

        // PIR
        pir_read(PIR_PIN, &pir);
        if (pir != 0) {
            pir_cnt++;
        }
        ble_peripheral_notify_pir(pir_cnt);
        Serial.print(", pir:");
        Serial.print(pir_cnt);
        pir_cnt = 0;

        Serial.println("");
        tmr_flags.clear(FLAG_TMR_AVAILABLE);
    }
}

void setup() {
    digitalWrite(LED_PWR, LOW);

    // initialize serial
    Serial.begin(115200);
    //  while (!Serial);

    Serial1.begin(9600);
    while (!Serial1)
        ;

    Wire.begin();

    // setup each device
    color_setup();
    bme280_setup(BME280_CS_PIN);
    co2_ndir_setup();
    pir_setup(PIR_PIN);

    // ble
    //  ble_peripheral_setup();

    // create sensor thread
    sensor_thread.start(sensor_thread_cb);

    // set flag
    tmr_flags.clear(FLAG_TMR_AVAILABLE);

    // start timer
    if (!(ITimer0.attachInterruptInterval(SAMPLING_PERIOD_US, timer_handler))) {
        Serial.println(F("Can't set ITimer0. Select another freq. or timer"));
    }
}

void loop() {
    //  ble_peripheral_loop();
}
