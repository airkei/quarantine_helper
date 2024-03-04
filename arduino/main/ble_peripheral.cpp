#include "ble_peripheral.h"

BLEService gestureService(GESTURE_SERVICE_UUID);  // create service
BLEByteCharacteristic gestureCharacteristic(GESTURE_CHARACTERISTIC_UUID,
                                            BLERead | BLENotify);

BLEDevice central;

void ble_peripheral_setup() {
    // begin initialization
    if (!BLE.begin()) {
        Serial.println("starting BLE failed!");

        while (1)
            ;
    }

    Serial.print("Local address is: ");
    Serial.println(BLE.address());

    // set the local name peripheral advertises
    BLE.setLocalName("GestureDetection");
    // set the UUID for the service this peripheral advertises:
    BLE.setAdvertisedService(gestureService);

    // add the characteristics to the service
    gestureService.addCharacteristic(gestureCharacteristic);

    // add the service
    BLE.addService(gestureService);

    gestureCharacteristic.writeValue(0);

    // start advertising
    BLE.advertise();

    Serial.println("Bluetooth device active, waiting for connections...");

    // wait for a BLE central
    while (true) {
        central = BLE.central();
        if (central) {
            break;
        }
        delay(500);
    }
    Serial.print("Connected to central: ");
    // print the central's BT address:
    Serial.println(central.address());
}

void ble_peripheral_loop() { BLE.poll(); }

bool ble_peripheral_notify(byte val) {
    gestureCharacteristic.writeValue(val);

    return true;
}
