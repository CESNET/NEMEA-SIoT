# BLE Connection Detector

## Description
This module detects when a BLE device is connected by monitoring advertising messages.

This detector receives information about advertising messages of adjacent BLE devices. For every device in reach is created a model for monitoring periodicity of its advertising messages. Every received advertisement is then compared to the model and if the time between two consecutive advertisements is longer a connection is detected and reported. The model is updated with every received advertisement from the given device.

The directory [tests](tests) is used for automatic integration tests to verify proper functionality and contains example inputs and expected corresponding outputs.

## Parameters
Usage: siot-ble-conn-detector [-hiIv] -i IFC_SPEC

### Module specific parameters
  - `-I`    Do not terminate module on incoming termination message

### Common TRAP parameters
  - `-h [trap,1]`      Print help message for this module / for libtrap specific parameters.
  - `-i IFC_SPEC`      Specification of interface types and their parameters.
  - `-v`               Be verbose.
  - `-vv`              Be more verbose.
  - `-vvv`             Be even more verbose.

## Interfaces
 - Input: 1
 - Output: 1

### Input UniRec format
	time    TIMESTAMP  - Timestamp of the device discovery
	macaddr DEV_ADDR   - Bluetooth address of the discovered device
	int8    RSSI       - Signal strength (distance) of the device
	uint8   ATYPE      - Type of the address [0 = public, 1 = random]

### Output UniRec format

	time    TIMESTAMP         - Timestamp of the first advertisement after the connection ended
	macaddr INCIDENT_DEV_ADDR - Bluetooth address of the connected device
	uint32  ALERT_CODE        - Reserved for future use [currently always 0x00]
	string  CAPTION           - Textual representation of report data
	uint8   ATYPE             - Type of the address [0 = public, 1 = random]
	uint32  DURATION          - Duration of connection in us. (Time between two consecutive advertisements.)
