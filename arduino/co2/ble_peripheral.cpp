#include "ble_peripheral.h"

BLEService sensorService(SENSOR_SERVICE_UUID);  // create service

BLEFloatCharacteristic temperatureCharacteristic(
    TEMPERATURE_CHARACTERISTIC_UUID, BLERead | BLENotify);
BLEFloatCharacteristic humidityCharacteristic(HUMIDITY_CHARACTERISTIC_UUID,
                                              BLERead | BLENotify);
BLEIntCharacteristic colorCharacteristic(COLOR_CHARACTERISTIC_UUID,
                                         BLERead | BLENotify);
BLEUnsignedShortCharacteristic vocCharacteristic(VOC_CHARACTERISTIC_UUID,
                                                 BLERead | BLENotify);
BLEUnsignedShortCharacteristic co2pasCharacteristic(CO2_PAS_CHARACTERISTIC_UUID,
                                                    BLERead | BLENotify);
BLEUnsignedShortCharacteristic co2ndirCharacteristic(
    CO2_NDIR_CHARACTERISTIC_UUID, BLERead | BLENotify);
BLEDoubleCharacteristic dustCharacteristic(DUST_CHARACTERISTIC_UUID,
                                           BLERead | BLENotify);
BLEUnsignedCharCharacteristic pirCharacteristic(PIR_CHARACTERISTIC_UUID,
                                                BLERead | BLENotify);
BLEFloatCharacteristic pressureCharacteristic(PRESSURE_CHARACTERISTIC_UUID,
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
    BLE.setLocalName("Sensors");
    // set the UUID for the service this peripheral advertises:
    BLE.setAdvertisedService(sensorService);

    // add the characteristics to the service
    sensorService.addCharacteristic(temperatureCharacteristic);
    sensorService.addCharacteristic(humidityCharacteristic);
    sensorService.addCharacteristic(colorCharacteristic);
    sensorService.addCharacteristic(vocCharacteristic);
    sensorService.addCharacteristic(co2pasCharacteristic);
    sensorService.addCharacteristic(co2ndirCharacteristic);
    sensorService.addCharacteristic(dustCharacteristic);
    sensorService.addCharacteristic(pirCharacteristic);
    sensorService.addCharacteristic(pressureCharacteristic);

    // add the service
    BLE.addService(sensorService);

    temperatureCharacteristic.writeValue(0);
    humidityCharacteristic.writeValue(0);
    colorCharacteristic.writeValue(0);
    vocCharacteristic.writeValue(0);
    co2pasCharacteristic.writeValue(0);
    co2ndirCharacteristic.writeValue(0);
    dustCharacteristic.writeValue(0);
    pirCharacteristic.writeValue(0);
    pressureCharacteristic.writeValue(0);

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

bool ble_peripheral_notify_temperature(float val) {
    temperatureCharacteristic.writeValue(val);

    return true;
}

bool ble_peripheral_notify_humidity(float val) {
    humidityCharacteristic.writeValue(val);

    return true;
}

bool ble_peripheral_notify_color(int val) {
    colorCharacteristic.writeValue(val);

    return true;
}

bool ble_peripheral_notify_voc(unsigned short val) {
    vocCharacteristic.writeValue(val);

    return true;
}

bool ble_peripheral_notify_co2_pas(unsigned short val) {
    co2pasCharacteristic.writeValue(val);

    return true;
}

bool ble_peripheral_notify_co2_ndir(unsigned short val) {
    co2ndirCharacteristic.writeValue(val);

    return true;
}

bool ble_peripheral_notify_dust(double val) {
    dustCharacteristic.writeValue(val);

    return true;
}

bool ble_peripheral_notify_pir(unsigned char val) {
    pirCharacteristic.writeValue(val);

    return true;
}

bool ble_peripheral_notify_pressure(float val) {
    pressureCharacteristic.writeValue(val);

    return true;
}
