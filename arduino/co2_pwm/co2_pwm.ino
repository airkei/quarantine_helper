#include <MsTimer2.h>
#include <Wire.h>

#include "bme280.h"
#include "pir.h"

#define BME280_CS_PIN 10
#define PIR_PIN 8
#define PWM_PIN A0

#define SAMPLING_PERIOD_MS (1000)
#define SAMPLING_TIMES 10

/*** Structure ***/

/*** Variables ***/
int sampling_cnt = 0;
uint16_t co2_ndir = 0;
long th, tl, h, l, ppm;
int prevVal = LOW;

/*** Interrupt ***/

/*** Functions ***/
static void sensor_thread_cb() {
    {
        static uint16_t prev_co2_ndir = 0;
        float temp = 0, hum = 0, pressure = 0;
        char pir = 0;
        static unsigned char pir_cnt = 0;

        sampling_cnt++;
        if (sampling_cnt < SAMPLING_TIMES) {
            // pir
            pir_read(PIR_PIN, &pir);
            if (pir != 0) {
                pir_cnt++;
            }
            return;
        } else {
            sampling_cnt = 0;
        }

        // Color
        Serial.print("color_a:0");

        // BME280
        bme280_read(&temp, &hum, &pressure);
        Serial.print(", temp:");
        Serial.print(temp);
        Serial.print(", hum:");
        Serial.print(hum);
        Serial.print(", pressure:");
        Serial.print(pressure);

        // CO2(NDIR)
        Serial.print(", co2:");
        Serial.print(co2_ndir);

        // PIR
        pir_read(PIR_PIN, &pir);
        if (pir != 0) {
            pir_cnt++;
        }
        Serial.print(", pir:");
        Serial.print(pir_cnt);
        pir_cnt = 0;

        Serial.println("");
    }
}

void setup() {
    // initialize serial
    Serial.begin(115200);
    //  while (!Serial);

    Wire.begin();

    // setup each device
    bme280_setup(BME280_CS_PIN);
    pir_setup(PIR_PIN);

    // start timer
    MsTimer2::set(SAMPLING_PERIOD_MS, sensor_thread_cb);
    MsTimer2::start();
}

void loop() {
    long tt = millis();
    int myVal = digitalRead(PWM_PIN);

    if (myVal == HIGH) {
        if (myVal != prevVal) {
            h = tt;
            tl = h - l;
            prevVal = myVal;
        }
    } else {
        if (myVal != prevVal) {
            l = tt;
            th = l - h;
            prevVal = myVal;
            co2_ndir = 5000 * (th - 2) / (th + tl - 4);
        }
    }
}
