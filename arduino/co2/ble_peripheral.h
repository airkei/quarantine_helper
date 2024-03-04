#ifndef __ble_peripheral_h__
#define __ble_peripheral_h__

#include <ArduinoBLE.h>

#define SENSOR_SERVICE_UUID "17bbb480-39fc-11ec-b844-039c3fbb664b"
#define TEMPERATURE_CHARACTERISTIC_UUID "17bbb481-39fc-11ec-b844-039c3fbb664b"
#define HUMIDITY_CHARACTERISTIC_UUID "17bbb482-39fc-11ec-b844-039c3fbb664b"
#define COLOR_CHARACTERISTIC_UUID "17bbb483-39fc-11ec-b844-039c3fbb664b"
#define VOC_CHARACTERISTIC_UUID "17bbb484-39fc-11ec-b844-039c3fbb664b"
#define CO2_PAS_CHARACTERISTIC_UUID "17bbb485-39fc-11ec-b844-039c3fbb664b"
#define CO2_NDIR_CHARACTERISTIC_UUID "17bbb486-39fc-11ec-b844-039c3fbb664b"
#define DUST_CHARACTERISTIC_UUID "17bbb487-39fc-11ec-b844-039c3fbb664b"
#define PIR_CHARACTERISTIC_UUID "17bbb488-39fc-11ec-b844-039c3fbb664b"
#define PRESSURE_CHARACTERISTIC_UUID "17bbb489-39fc-11ec-b844-039c3fbb664b"

void ble_peripheral_setup();
void ble_peripheral_loop();
bool ble_peripheral_notify_temperature(float val);
bool ble_peripheral_notify_humidity(float val);
bool ble_peripheral_notify_color(int val);
bool ble_peripheral_notify_voc(unsigned short val);
bool ble_peripheral_notify_co2_pas(unsigned short val);
bool ble_peripheral_notify_co2_ndir(unsigned short val);
bool ble_peripheral_notify_dust(double val);
bool ble_peripheral_notify_pir(unsigned char val);
bool ble_peripheral_notify_pressure(float val);

#endif
