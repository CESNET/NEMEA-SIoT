# BLE Advertising Scanner

## Basic info
Author: Ondřej Hujňák <hujnak@cesnet.cz>

Goal: Provide BLE devices which are in proximity of the BLE controller.

Inputs:  --
Outputs: UniRec [macaddr BDADDR, time TIME, int8 RSSI, uint8 ATYPE]

Usage: python3 ble_adv_scanner.py -i IFC_SPEC

Example: python3 ble_adv_scanner.py -i "u:socket"

## Module description
Module scans advertising channels of Bluetooth Low Energy and every discovered
device is reported to UniRec output. For scanning is used any (standard) BLE
controller, which communicates via HCI. Scanning itself is by default passive,
so no packets are sent out and all reported devices are discovered by cyclic
listening to advertising channels.

## UniRec output format
	macaddr BDADDR	- Bluetooth address of the discovered device
	time	TIME	- Timestamp of the device discovery
	int8	RSSI	- Signal strength (distance) of the device
	uint8	ATYPE	- Type of the address [0 = public, 1 = random]
