import argparse
import csv
from datetime import datetime as dt
import sys
import threading
import re

import boto3
import serial

SERIAL_DEVICE1_NAME = "/dev/tty.usbmodem1141101"
SERIAL_DEVICE2_NAME = "/dev/tty.usbmodem1141201"


class Scanner():
    def __init__(self):
        self.initial_flag = True
        self.exit_flag = False
        self.mph = []
        self.door_state = "close"
        self.pos_state = "0"

    def window_run(self):
        try:
            ser = serial.Serial(SERIAL_DEVICE1_NAME, 115200, timeout=None)
        except serial.serialutil.SerialException:
            print("No Window information")
            return

        door_close_cnt = 0
        while True:
            line = ser.readline().decode().replace('\r', '').replace('\n', '')
            if not line.startswith("[DATA]"):
                continue

            # extract data
            try:
                mph_str = re.search(r'mph:([\d\.]+)', line)
                self.mph.append(float(mph_str.group(1)))
            except AttributeError:
                self.mph.append(0)

            door_str = re.search(r'door:(\d+)', line)
            door = int(door_str.group(1))
            if door == 0:
                door_close_cnt = 0
                self.door_state = "open"
            else:
                door_close_cnt += 1
                if door_close_cnt >= 10:
                    self.door_state = "close"
                else:
                    self.door_state = "open"

            pos0_str = re.search(r'pos0:(\d+)', line)
            pos0 = int(pos0_str.group(1))
            pos1_str = re.search(r'pos1:(\d+)', line)
            pos1 = int(pos1_str.group(1))
            pos2_str = re.search(r'pos2:(\d+)', line)
            pos2 = int(pos2_str.group(1))
            pos3_str = re.search(r'pos3:(\d+)', line)
            pos3 = int(pos3_str.group(1))

            if pos0 + pos1 + pos2 + pos3 == 1:
                if pos0 == 1:
                    self.pos_state = "0"
                elif pos1 == 1:
                    self.pos_state = "1"
                elif pos2 == 1:
                    self.pos_state = "2"
                elif pos3 == 1:
                    self.pos_state = "3"

            if self.exit_flag:
                break
        ser.close()

    def run(self, conditioner, ventilation, fan):
        ser = serial.Serial(SERIAL_DEVICE2_NAME, 115200, timeout=None)
        stime = dt.now()

        while True:
            line = ser.readline().decode().replace('\r', '').replace('\n', '')

            # write csv header
            if self.initial_flag:
                header_str = re.sub(r'[^a-zA-Z\_\,]', '', line)
                header_arr = ["dt", "color", "temp", "hum", "pressure", "co2",
                              "pir", "mph", "door", "pos", "conditioner", "ventilation", "fan"]
                with open('/tmp/data_air.csv', 'w') as f:
                    writer = csv.writer(f)
                    writer.writerow(header_arr)
                self.initial_flag = False

            # write csv row
            tdatetime = dt.now()
            tstr = tdatetime.strftime('%Y/%m/%d %H:%M:%S')
            color_str = re.search(r'color_a:(\d+)', line)
            color = int(color_str.group(1))
            temp_str = re.search(r'temp:([\d\.]+)', line)
            temp = float(temp_str.group(1))
            hum_str = re.search(r'hum:([\d\.]+)', line)
            hum = float(hum_str.group(1))
            pressure_str = re.search(r'pressure:([\d\.]+)', line)
            pressure = float(pressure_str.group(1))
            co2_str = re.search(r'co2:(\d+)', line)
            co2 = int(co2_str.group(1))
            pir_str = re.search(r'pir:(\d+)', line)
            pir = int(pir_str.group(1))

            try:
                mph_ave = sum(self.mph) / len(self.mph)
            except ZeroDivisionError:
                mph_ave = 0
            self.mph = list()
            data_arr = [tstr, color, temp, hum, pressure, co2, pir, mph_ave,
                        self.door_state, self.pos_state, conditioner, ventilation, fan]
            print(data_arr)
            with open('/tmp/data_air.csv', 'a') as f:
                writer = csv.writer(f)
                writer.writerow(data_arr)

            if self.exit_flag:
                break
        # upload to S3
        print("uploading to S3 ...")
        s3 = boto3.resource('s3')
        bucket = s3.Bucket('co2-data-keisuke-nakata')

        daystr = stime.strftime('%Y/%m/%d')
        secstr = stime.strftime('%H-%M-%S')
        key_name = 'dt=' + daystr + '/' + secstr + '.csv'
        bucket.upload_file('/tmp/data_air.csv', key_name)

        key_name = 'latest/data.csv'
        bucket.upload_file('/tmp/data_air.csv', key_name)
        print("done")

        ser.close()

    def check_stdin(self):
        while True:
            ch = input()
            if ch == 'q':
                self.exit_flag = True
                break


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='sudo python co2_upload.py (conditioner) (ventilation) (fan)')

    parser.add_argument('-c', '--conditioner', required=False,
                        default='on', help='conditioner state')
    parser.add_argument('-v', '--ventilation', required=False,
                        default='on', help='ventilation state')
    parser.add_argument('-fan', '--fan', required=False,
                        default='on', help='fan state')

    args = parser.parse_args()

    print("Press q and ENTER to stop")
    pl = Scanner()

    # start plot and creating csv, then upload to S3
    th_stdin = threading.Thread(target=pl.check_stdin)
    th_stdin.start()

    th_window = threading.Thread(target=pl.window_run)
    th_window.start()

    try:
        pl.run(args.conditioner, args.ventilation, args.fan)
    except KeyboardInterrupt:
        print("Press q and ENTER to stop")
