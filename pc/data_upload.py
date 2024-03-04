import csv
from datetime import datetime as dt
import sys
import threading
import re

import boto3
import matplotlib.pyplot as plt
import serial

SERIAL_DEVICE_NAME = "/dev/tty.usbmodem1141301"


class Plotter():
    def __init__(self, label, samples):
        self.initial_flag = True
        self.exit_flag = False
        self.label = label
        self.samples = samples
        self.tdatetime = dt.now()

    def run(self):
        fig, ax = plt.subplots()

        arr0 = [0] * self.samples
        arr1 = [0] * self.samples
        arr2 = [0] * self.samples
        arr3 = [0] * self.samples
        arr4 = [0] * self.samples
        arr5 = [0] * self.samples
        arr6 = [0] * self.samples
        arr7 = [0] * self.samples
        x = range(0, self.samples)

        ser = serial.Serial(SERIAL_DEVICE_NAME, 115200, timeout=None)

        tstr = self.tdatetime.strftime('%Y/%m/%d %H:%M:%S')
        disp_cnt = 0
        while True:
            while True:
                line = ser.readline().decode().replace('\r', '').replace('\n', '')
                if not line.startswith("[DATA]"):
                    #                    print(line)
                    continue

                # extract emg data
                cnt_str = re.search(r'cnt:(\d+)', line)
                cnt = int(cnt_str.group(1))
                emg0 = re.search(r'emg0:([\d\.]+)', line)
                val0 = float(emg0.group(1))
                emg1 = re.search(r'emg1:([\d\.]+)', line)
                val1 = float(emg1.group(1))
                acc_x = re.search(r'acc_x:([\d\.]+)', line)
                val2 = float(acc_x.group(1))
                acc_y = re.search(r'acc_y:([\d\.]+)', line)
                val3 = float(acc_y.group(1))
                acc_z = re.search(r'acc_z:([\d\.]+)', line)
                val4 = float(acc_z.group(1))
                gyro_x = re.search(r'gyro_x:([\d\.]+)', line)
                val5 = float(gyro_x.group(1))
                gyro_y = re.search(r'gyro_y:([\d\.]+)', line)
                val6 = float(gyro_y.group(1))
                gyro_z = re.search(r'gyro_z:([\d\.]+)', line)
                val7 = float(gyro_z.group(1))

                # write csv header
                if self.initial_flag:
                    if cnt != 0:
                        continue

                    line = line.lstrip('[DATA]')
                    header_str = re.sub(r'[^a-zA-Z\_\,]', '', line)
                    header_arr = header_str.split(',')
                    header_arr.insert(0, "dt")
                    header_arr.append("label")
                    with open('/tmp/data.csv', 'w') as f:
                        writer = csv.writer(f)
                        writer.writerow(header_arr)
                    self.initial_flag = False

                # write csv row
                data_arr = re.findall(r'[\d\.]+', line)
                data_arr.insert(0, tstr)
                data_arr.append(label)
                with open('/tmp/data.csv', 'a') as f:
                    writer = csv.writer(f)
                    writer.writerow(data_arr)

                arr0[cnt] = val0
                arr1[cnt] = val1
                arr2[cnt] = val2
                arr3[cnt] = val3
                arr4[cnt] = val4
                arr5[cnt] = val5
                arr6[cnt] = val6
                arr7[cnt] = val7
                if cnt == self.samples-1:
                    break

            print(disp_cnt)
            disp_cnt += 1

            # plot
            ax.set_xlim(0, self.samples)
#            ax.set_ylim(400, 1024)
            line0, = ax.plot(x, arr0, color='blue')
            line1, = ax.plot(x, arr1, color='green')
            line2, = ax.plot(x, arr2, color='red')
            line3, = ax.plot(x, arr3, color='tomato')
            line4, = ax.plot(x, arr4, color='lightsalmon')
#            line5, = ax.plot(x, arr5, color='black')
#            line6, = ax.plot(x, arr6, color='grey')
#            line7, = ax.plot(x, arr7, color='silver')
            plt.pause(0.1)
            line0.remove()
            line1.remove()
            line2.remove()
            line3.remove()
            line4.remove()
#            line5.remove()
#            line6.remove()
#            line7.remove()

            if self.exit_flag:
                break

        # upload to S3
        print("uploading to S3 ...")
        s3 = boto3.resource('s3')
        bucket = s3.Bucket('sensor-data-keisuke-nakata')

        daystr = self.tdatetime.strftime('%Y/%m/%d')
        secstr = self.tdatetime.strftime('%H-%M-%S')
        key_name = 'sample=' + str(self.samples) + '/label=' + \
            self.label + '/dt=' + daystr + '/' + secstr + '.csv'
        bucket.upload_file('/tmp/data.csv', key_name)

        key_name = 'sample=' + str(self.samples) + \
            '/label=' + self.label + '/latest/data.csv'
        bucket.upload_file('/tmp/data.csv', key_name)
        print("done")
        ser.close()

    def check_stdin(self):
        while True:
            ch = input()
            if ch == 'q':
                self.exit_flag = True
                break


if __name__ == '__main__':
    if len(sys.argv) == 2:
        label = sys.argv[1]
        samples = 64
    elif len(sys.argv) == 3:
        label = sys.argv[1]
        samples = int(sys.argv[2])
    else:
        print("usage: sudo python data_plot.py label (samples)")
        exit(0)
    print("Press q and ENTER to stop")

    pl = Plotter(label, samples)
    # start plot and creating csv, then upload to S3
    th = threading.Thread(target=pl.check_stdin)
    th.start()
    try:
        pl.run()
    except KeyboardInterrupt:
        print("Press q and ENTER to stop")
