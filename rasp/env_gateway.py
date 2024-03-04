import argparse
import struct
import sys
from time import gmtime, strftime, sleep
from datetime import datetime as dt
from bluepy import btle
from bluepy.btle import Scanner, DefaultDelegate, BTLEException
import publish

SENSOR_SERVICE_UUID = "17bbb480-39fc-11ec-b844-039c3fbb664b"
TEMPERATURE_CHARACTERISTIC_UUID = "17bbb481-39fc-11ec-b844-039c3fbb664b"
HUMIDITY_CHARACTERISTIC_UUID = "17bbb482-39fc-11ec-b844-039c3fbb664b"
COLOR_CHARACTERISTIC_UUID = "17bbb483-39fc-11ec-b844-039c3fbb664b"
VOC_CHARACTERISTIC_UUID = "17bbb484-39fc-11ec-b844-039c3fbb664b"
CO2_PAS_CHARACTERISTIC_UUID = "17bbb485-39fc-11ec-b844-039c3fbb664b"
CO2_NDIR_CHARACTERISTIC_UUID = "17bbb486-39fc-11ec-b844-039c3fbb664b"
DUST_CHARACTERISTIC_UUID = "17bbb487-39fc-11ec-b844-039c3fbb664b"
PIR_CHARACTERISTIC_UUID = "17bbb488-39fc-11ec-b844-039c3fbb664b"
PRESSURE_CHARACTERISTIC_UUID = "17bbb489-39fc-11ec-b844-039c3fbb664b"

devaddr = None


def prettify(mac_string):
    return ':'.join('%02x' % b for b in mac_string)


def publish_routine(peri):
    pub = publish.Publisher(endpoint=args.ep, cid='Gateway',
                            cert=args.cert, key=args.key, root=args.root)
    pub.connect()
    while True:
        dict_data = {}
        now = dt.now()
        dict_data['date'] = now.strftime('%Y-%m-%d %H:%M:%S')
        dict_data['address'] = devaddr
        dict_data['room'] = args.position
        charas = peri.getCharacteristics()
        for chara in charas:
            if chara.uuid == TEMPERATURE_CHARACTERISTIC_UUID:
                dict_data['temperature'] = struct.unpack('<f', chara.read())[0]
            elif chara.uuid == HUMIDITY_CHARACTERISTIC_UUID:
                dict_data['humidity'] = struct.unpack('<f', chara.read())[0]
            elif chara.uuid == COLOR_CHARACTERISTIC_UUID:
                dict_data['ambient'] = int.from_bytes(chara.read(), 'little')
            elif chara.uuid == VOC_CHARACTERISTIC_UUID:
                dict_data['voc'] = int.from_bytes(chara.read(), 'little')
            elif chara.uuid == CO2_PAS_CHARACTERISTIC_UUID:
                dict_data['co2_pas'] = int.from_bytes(chara.read(), 'little')
            elif chara.uuid == CO2_NDIR_CHARACTERISTIC_UUID:
                dict_data['co2_ndir'] = int.from_bytes(chara.read(), 'little')
            elif chara.uuid == DUST_CHARACTERISTIC_UUID:
                dict_data['dust'] = struct.unpack('<d', chara.read())[0]
            elif chara.uuid == PIR_CHARACTERISTIC_UUID:
                dict_data['pir'] = int.from_bytes(chara.read(), 'little')
            elif chara.uuid == PRESSURE_CHARACTERISTIC_UUID:
                dict_data['pressure'] = struct.unpack('<f', chara.read())[0]

        print(dict_data)
        topic = 'device/' + devaddr.replace(':', '') + '/envdata'
        pub.publish(topic=topic, dict_data=dict_data)
        sleep(10)
    pub.disconnect()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description='sudo python3 env_gateway.py cert key root ep position')

    parser.add_argument('-c', '--cert', required=True, help='cert file path')
    parser.add_argument('-k', '--key', required=True, help='key file path')
    parser.add_argument('-r', '--root', required=True, help='root file path')
    parser.add_argument('-e', '--ep', required=True, help='endpoint')
    parser.add_argument('-p', '--position', required=True, help='position')

    args = parser.parse_args()

    while True:
        try:
            scanner = btle.Scanner(0)
            devices = scanner.scan(1.0, passive=True)
            for device in devices:
                for (adtype, desc, value) in device.getScanData():
                    if value != SENSOR_SERVICE_UUID:
                        continue
                    print(' (%3s) %s : %s ' % (adtype, desc, value))
                    # try to connect
                    peri = btle.Peripheral()
                    peri.connect(device.addr, btle.ADDR_TYPE_PUBLIC)
                    devaddr = device.addr
                    print('connected: ' + device.addr)
            if devaddr is None:
                continue

            # check handle
            publish_routine(peri)

        except BTLEException:
            print('BTLE Exception while scannning.')
