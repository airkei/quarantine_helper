#ifndef __ble_peripheral_h__
#define __ble_peripheral_h__

#include <ArduinoBLE.h>

#define GESTURE_SERVICE_UUID "c14debb0-38a7-11ec-ba9c-93b3f272a4e7"
#define GESTURE_CHARACTERISTIC_UUID "c14debb1-38a7-11ec-ba9c-93b3f272a4e7"

void ble_peripheral_setup();
void ble_peripheral_loop();
bool ble_peripheral_notify(byte val);

#endif
