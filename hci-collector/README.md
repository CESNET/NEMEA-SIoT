# README
# Bluetooth HCI Collector

## Description
Collects all Bluetooth packets flowing through the specified Bluetooth HCI
and send them out with their type, timestamp, size and data direction.
For ACL data packets, also device address is sent (00:00:00:00:00:00 otherwise).

It requires an installed library libbluetooth and it must be run with root privileges.

### Debian based systems
- libbluetooth-dev `apt install libbluetooth-dev`

### OpenWrt
- bluez-libs `opkg install bluez-libs`

## Output Unirec Interface
	time    TIMESTAMP
	macaddr DEV_ADDR       - device address (for ACL data packets)
	macaddr HCI_DEV_ADDR   - hci device address
	uint8   PACKET_TYPE    - command / event / ACL data / SCO data
	uint8   DATA_DIRECTION - 0 / 1
	uint8   SIZE           - packet size
	bytes   PACKET         - variable length packet

## Parameters
### Module specific parameters
- `-d  --dev <int>`  Defines HCI to sniff from (default 0 etc.).

### Common TRAP parameters
- `-h [trap,1]`      Print help message for this module / for libtrap specific parameters.
- `-i IFC_SPEC`      Specification of interface types and their parameters.
- `-v`               Be verbose.
- `-vv`              Be more verbose.
- `-vvv`             Be even more verbose.
