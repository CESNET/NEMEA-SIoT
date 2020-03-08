# Tests for LoRaWAN Distance module
All test input datasets contains UniRec records which simulates LoRaWAN traffic captured by [LoRa Collector](https://github.com/CESNET/NEMEA-SIoT/tree/master/lora-collector).

##  Device moved - device-move
Detection is for fixed-position devices, if the attacker transfers the device, the RSSI (Received Signal Strength Indication) changes. 

Input Traffic: device-move.csv

Set variance on 5% (0.05) [-a --variance <double>].

### Input
Test input dataset from traffic produced by a single LoRaWAN gateway. File contain 118 network packet records.

### Expected output
An alert messages containing 5 messages, that the device has probably changed its position.

## Device did not move - device-move
Detection is for fixed-position devices, if the attacker transfers the device, the RSSI (Received Signal Strength Indication) changes. 

Input Traffic: device-move.csv

Set default value on 10% (0.1) [-a --variance <double>].

### Input
Test input dataset from traffic produced by a single LoRaWAN gateway. File contain 118 network packet records.

### Expected output
Output is empty, devices not detected.
