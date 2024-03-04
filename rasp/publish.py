# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: MIT-0

import argparse
from awscrt import io, mqtt, auth, http, exceptions
from awsiot import mqtt_connection_builder
import time as t
import json


class Publisher():
    def __init__(self, endpoint, cid, cert, key, root):
        self.endpoint = endpoint
        self.cid = cid
        self.cert = cert
        self.key = key
        self.root = root
        self.mqtt_connection = None

    def connect(self):
        # Spin up resources
        event_loop_group = io.EventLoopGroup(1)
        host_resolver = io.DefaultHostResolver(event_loop_group)
        client_bootstrap = io.ClientBootstrap(event_loop_group, host_resolver)
        self.mqtt_connection = mqtt_connection_builder.mtls_from_path(
            endpoint=self.endpoint,
            cert_filepath=self.cert,
            pri_key_filepath=self.key,
            client_bootstrap=client_bootstrap,
            ca_filepath=self.root,
            client_id=self.cid,
            clean_session=False,
            keep_alive_secs=6
        )
        #    print("Connecting to {} with client ID '{}'...".format(self.endpoint, self.cid))
        # Make the connect() call
        connect_future = self.mqtt_connection.connect()
        # Future.result() waits until a result is available
        connect_future.result()
        #    print("Connected!")

    def publish(self, topic, dict_data):
        # Publish message to server desired number of times.
        #   print('Begin Publish')
        self.mqtt_connection.publish(topic=topic, payload=json.dumps(
            dict_data), qos=mqtt.QoS.AT_LEAST_ONCE)
        print("Published: '" + json.dumps(dict_data) + "' to the topic: " + topic)

    def disconnect(self):
        disconnect_future = self.mqtt_connection.disconnect()
        disconnect_future.result()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description='python3 publish.py cert key root data topic endpoint')

    parser.add_argument('-c', '--cert', required=True, help='cert file path')
    parser.add_argument('-k', '--key', required=True, help='key file path')
    parser.add_argument('-r', '--root', required=True, help='root file path')
    parser.add_argument('-d', '--data', required=True, help='data file path')
    parser.add_argument('-t', '--topic', required=True, help='topic name')
    parser.add_argument('-e', '--ep', required=True, help='endpoint')

    args = parser.parse_args()

    # open data file(json)
    with open(args.data) as f:
        dict_data = json.load(f)

    pub = Publisher(endpoint=args.ep, cid='Gateway',
                    cert=args.cert, key=args.key, root=args.root)

    pub.connect()
    pub.publish(topic=args.topic, dict_data=dict_data)
    pub.disconnect()
