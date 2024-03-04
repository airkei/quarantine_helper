import argparse
import time
import threading
from beacontools_modified.beacontools import BeaconScanner, IBeaconFilter, IBeaconAdvertisement


class iBeacon():
    def __init__(self, verbose=False, uuid='00000000-e132-1001-b000-001c4de2af03', interval=1):
        self.init = True
        self.verbose = verbose
        self.uuid = str.lower(uuid)
        self.interval = interval

        self.ibeacon_dict = {}
        self.current_dict = {}

        self.scanner = BeaconScanner(self.callback,
                                     device_filter=IBeaconFilter(
                                         uuid=self.uuid)
                                     )

    def get_info(self):
        return self.current_dict

    def update_dict(self):
        while True:
            if self.verbose:
                print(self.current_dict)

            self.current_dict = self.ibeacon_dict
            self.ibeacon_dict = {}
            time.sleep(self.interval)

    def callback(self, bt_addr, rssi, packet, additional_info):
        #    print("<%s, %d> %s %s" % (bt_addr, rssi, packet, additional_info))
        self.ibeacon_dict[bt_addr] = (rssi, additional_info)

    def exec_scan(self):
        self.ibeacon_dict = {}
        if self.init:
            self.scanner.start()
            self.init = False

        time.sleep(self.interval)

        return self.ibeacon_dict

    def start_scan(self):
        self.scanner.start()

        thread = threading.Thread(target=self.update_dict)
        thread.start()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description='sudo python3 ibscanner.py uuid  interval')

    parser.add_argument(
        '-u', '--uuid', default='00000000-e132-1001-b000-001c4de2af03', help='uuid')
    parser.add_argument('-i', '--interval', default=1, help='interval(sec)')

    args = parser.parse_args()

    ib = iBeacon(verbose=True, uuid=args.uuid, interval=int(args.interval))
    ib.start_scan()
