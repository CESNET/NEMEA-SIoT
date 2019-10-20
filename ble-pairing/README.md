# README
# BLE Pairing Detector

## Description
BLE Pairing Detector is intended to find unexpected BLE pairing, which may indicate
an attack on the pairing. It receives output of the Bluetooth HCI Collector,
looks for a pairing process and generates alerts.

Paired devices are stored in the specified directory as HCI_DEV_ADDR-DEV_ADDR.

It requires an installed library libbluetooth.

### Debian based systems
- libbluetooth-dev `apt install libbluetooth-dev`

### OpenWrt
- bluez-libs `opkg install bluez-libs`

## Input Unirec Interface
	time    TIMESTAMP
	macaddr DEV_ADDR       - device address (for ACL data packets)
	macaddr HCI_DEV_ADDR   - hci device address
	uint8   PACKET_TYPE    - command / event / ACL data / SCO data
	uint8   DATA_DIRECTION - 0 / 1
	uint8   SIZE           - packet size
	bytes   PACKET         - variable length packet

## Output Unirec Interface
	time    TIMESTAMP
	macaddr INCIDENT_DEV_ADDR  - pairing device address
	uint32  ALERT_CODE         - repeated pairing (0 / 1)
	string  CAPTION            
	macaddr HCI_DEV_ADDR       - hci device address
	uint8   SUCCESS            - pairing was successful (1 / 0)
	uint8   VERSION            - BLE 4.0, 4.1 (0) / BLE 4.2+ (1) / UNKNOWN (255)
	uint8   METHOD             - JUST WORKS (0) / PASSKEY_ENTRY (1) / OOB (2) / NUMERIC_COMPARISON (3) / UNKNOWN (255)

## Parameters
### Module specific parameters
- `-d  --dir <string>`  Defines directory to store paired devices.

### Common TRAP parameters
- `-h [trap,1]`      Print help message for this module / for libtrap specific parameters.
- `-i IFC_SPEC`      Specification of interface types and their parameters.
- `-v`               Be verbose.
- `-vv`              Be more verbose.
- `-vvv`             Be even more verbose.
