# BLE Connection Guard

## Description
This module allows monitoring of BLE connection to the devices.

The detector receives connection reports from [ble-conn-detector](https://github.com/CESNET/NEMEA-SIoT/tree/master/ble-conn-detector) and filters them according to the configuration and dynamic setting. It allows filtering the reports by Bluetooth address and time, or can be configured fo manual trigger of reporting for a device.

The directory [tests](tests) is used for automatic integration tests to verify proper functionality and contains example inputs and expected corresponding outputs.

## Parameters
Usage: siot-ble-conn-guard [-hIv] [-c <config_file>] -i IFC_SPEC

### Module specific parameters
  - `-c <config_file>` Uses specified configuration file. Default is ./ble-conn-guard.ini.
  - `-I`               Do not terminate module on incoming termination message

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
	time    TIMESTAMP         - Timestamp of the first advertisement after the connection ended
	macaddr INCIDENT_DEV_ADDR - Bluetooth address of the connected device
	uint32  ALERT_CODE        - Reserved for future use [currently always 0x00]
	string  CAPTION           - Textual representation of report data
	uint8   ATYPE             - Type of the address [0 = public, 1 = random]
	uint32  DURATION          - Duration of connection in us. (Time between two consecutive advertisements.)

### Output UniRec format

	time    TIMESTAMP         - Timestamp of the first advertisement after the connection ended
	macaddr INCIDENT_DEV_ADDR - Bluetooth address of the connected device
	uint32  ALERT_CODE        - Reserved for future use [currently always 0x00]
	string  CAPTION           - Textual representation of report data
	uint8   ATYPE             - Type of the address [0 = public, 1 = random]
	uint32  DURATION          - Duration of connection in us. (Time between two consecutive advertisements.)
