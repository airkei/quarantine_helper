#import argparse
from time import gmtime, strftime, sleep
from datetime import datetime as dt
#import threading
#from beacontools_modified import BeaconScanner, IBeaconFilter, IBeaconAdvertisement
import ibscanner
import csv
import pandas as pd
import numpy as np

if __name__ == "__main__":

    ib = ibscanner.iBeacon()
    interval = 0.1
    max_count = 100

    dt_now = dt.now()
    dt_str = "{0:%Y%m%d%H%M%S}".format(dt_now)
    logfilename = 'beacon_rcv_data'+dt_str+'.csv'
    print(logfilename + ' -------------------------')
    headers = ['date_time', 'number_of_beacon',
               'address', 'rssi', 'major', 'minor', 'uuid']

    if max_count > 0:
        with open(logfilename, 'w') as f:
            writer = csv.writer(f, lineterminator='\n')
            writer.writerow(headers)

    while max_count > 0:
        dt_now = dt.now()
        ibnfo = ib.exec_scan()
        print('number_of_beacon:', len(ibnfo))
        print('date_time', dt_now)
        print()

        for k, v in ibnfo.items():
            binfo = {}
            binfo['address'] = k
            binfo['rssi'] = v[0]
            binfo['major'] = v[1]['major']
            binfo['minor'] = v[1]['minor']
            binfo['uuid'] = v[1]['uuid']
            # dict_data['beaon'].append(binfo)
            print('address:', binfo['address'])
            print('rssi', binfo['rssi'])
            print('major', binfo['major'])
            print('minor', binfo['minor'])
            print('uuid', binfo['uuid'])
            print()

            with open(logfilename, 'a') as f:
                writer = csv.writer(f, lineterminator='\n')
                writer.writerow([dt_now, len(ibnfo), binfo['address'],
                                binfo['rssi'], binfo['major'], binfo['minor'], binfo['uuid']])

        max_count = max_count - 1
        sleep(interval)
