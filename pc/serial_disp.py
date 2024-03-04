# coding: utf-8

import serial
import re

SERIAL_DEVICE_NAME = "/dev/tty.usbmodem1141301"


def run():
    ser = serial.Serial(SERIAL_DEVICE_NAME, 115200, timeout=None)
    pattern = '.*?(\d+)\(\d+)\%\)'
    while True:
        line = ser.readline().decode().replace('\r', '').replace('\n', '')
        if not line.startswith("[BLE]"):
            # print(line)
            continue
        act = int(re.findall(r'\d+', line)[0])
        prob = re.findall(r'\d+\.\d+', line)[0]
        if act == 0:  # None
            print('無状態' + '(' + prob + '%)')
        elif act == 1:  # Grab
            print('\033[31m' + '掴む' + '(' + prob + '%)' + '\033[0m')
        elif act == 2:  # Handle
            print('\033[32m' + '握る' + '(' + prob + '%)' + '\033[0m')
        elif act == 3:  # Door
            print('\033[33m' + 'ドア' + '(' + prob + '%)' + '\033[0m')
        elif act == 4:  # Lock
            print('\033[34m' + 'ロック' + '(' + prob + '%)' + '\033[0m')
    ser.close()


if __name__ == '__main__':
    run()
