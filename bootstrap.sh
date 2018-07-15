#!/bin/bash
autoreconf -i
cd ble-pairing
autoreconf -i
cd ../hci-collector
autoreconf -i
cd ../wsn-anomaly/detector
autoreconf -i
cd ../../lora-airtime
autoreconf -i
cd ../lora-replay
autoreconf -i
