# Tests for module LoRa Distance
    All test input datasets contains UniRec records which simulates LoRaWAN
    traffic captured by [LoRa Collector](https://github.com/CESNET/NEMEA-SIoT/tree/master/lora-collector).

## device-moved

Set default value on 10% (0.1).

### input: 
    Test input dataset simulates traffic produced by a single LoRaWAN node. 
    First 5 messages contain simular RSSI values. Following 5 messages contain 
    significantly different RSSI value.

### expected output:
    An allert message containing information, that the device has probably changed
    its position.

## device-did-not-move

Set default value on 10% (0.1).

### input: 
    Test input dataset simulates traffic between LoRa Gateway and a single LoRa
    end-device captured by LoRa Collector (for more information see 
    [https://github.com/CESNET/NEMEA-SIoT/tree/lora-test/lora-distance]). 
    First 5 messages contain simular RSSI values. Following 5 messages contain 
    significantly different RSSI value.

### expected output:
    Empty.
