# BLE Advertising Scanner

## Basic info
Author: Ondřej Hujňák <hujnak@cesnet.cz>

Goal: Provide all advertising packets from the proximity of the BLE controller.

	Inputs:  --
	Outputs: UniRec [time TIMESTAMP, macaddr DEV_ADDR, uint8 ATYPE, int8 RSSI]

Usage: siot-ble-adv-collector [-d/--dev hci_device_id] -i IFC_SPEC

Example: siot-ble-adv-collector -i "u:ble-adv-collector"

## Module description
Module scans advertising channels of Bluetooth Low Energy and every discovered
device is reported to UniRec output. For scanning is used any (standard) BLE
controller, which communicates via HCI. Scanning itself is by default passive,
so no packets are sent out and all reported devices are discovered by cyclic
listening to advertising channels.

## UniRec output format
	time    TIMESTAMP  - Timestamp of the device discovery
	macaddr DEV_ADDR   - Bluetooth address of the discovered device
	int8    RSSI       - Signal strength (distance) of the device
	uint8   ATYPE      - Type of the address [0 = public, 1 = random]
