# README
# Bluetooth HCI Collector

## Description
Collects all Bluetooth packets flowing through the specified Bluetooth HCI
and send them out with their type, timestamp and data direction.

It requires an installed library libbluetooth and it must be run with root privileges.

### Debian based systems
- libbluetooth-dev `apt install libbluetooth-dev`

### OpenWrt
- bluez-libs `opkg install bluez-libs`

## Interfaces
- Output: One UniRec interface (template contains these fields):
  - HCI_DEV_MAC    - hci device address
  - TIMESTAMP
  - DATA_DIRECTION - 0 / 1
  - PACKET_TYPE    - command / event / ACL data / SCO data
  - PACKET         - variable length

## Parameters
### Module specific parameters
- `-d  --dev <int>`  Defines HCI to sniff from (default 0 etc.).

### Common TRAP parameters
- `-h [trap,1]`      Print help message for this module / for libtrap specific parameters.
- `-i IFC_SPEC`      Specification of interface types and their parameters.
- `-v`               Be verbose.
- `-vv`              Be more verbose.
- `-vvv`             Be even more verbose.
