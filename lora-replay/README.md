## THIS MODULE IS CURRENTLY UNDER DEVELOPMENT
---

---
# README
# LoRaWAN Detection - Replay attack ABP
This detector serves for detection replay attack in LoRaWAN infrastructure of ABP authentication method.

The attacker is detected based on its behavior. The attack process begins with data storage of each device captured within range. If an attacker captures a restart message for the device (Fcnt = 0), sends the last stored message to the gateway from captured data where is device with the highest counter value. The attack replicates the last message and sends it over the gateway to the server.Server assumes that it receives a higher message than the attacker has sent and awaits this message. Sensor reboot and sending messages until it reaches the counter of the attacking message. Sensor is dosing for this time. This attack can be harmful for ABP activated end devices.

Attack detection is performed by saving data to the list (DeviceList). Information is retrieved from incoming physical payload (PHYPayload) by parsing and revers octets. Each row in DeviceList contains device has a counter (FCnt) of received message and information about restart the device (RESTART). If the device is restarted, RESTART value is set to 1. The device address (DevAddr) is used as the row index. An attacker is recognized if his last message is the same as the message after restarting the device. Identification of the attacker is based on same couture (FCnt).

## Description

## Interfaces
- Input: One UniRec interface
Template must contain fields TIMESTAMP, PHY_PAYLOAD.

- Output: One UniRec interface
Template contain this fields DEV_ADDR, TIMESTAMP, FCNT.
  
## Parameters
### Common TRAP parameters
- `-h [trap,1]`      Print help message for this module / for libtrap specific parameters.
- `-i IFC_SPEC`      Specification of interface types and their parameters.
- `-v`               Be verbose.
- `-vv`              Be more verbose.
- `-vvv`             Be even more verbose.
