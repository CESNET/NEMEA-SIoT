## THIS MODULE IS CURRENTLY UNDER DEVELOPMENT
---

---
# README
# LoRaWAN Detection - Replay attack ABP
This detector serves for detection replay attack in LoRaWAN infrastructure of ABP authentication method.

The attacker is detected based on its behavior. The attack process begins with data storage of each device captured within range. If an attacker captures a restart message for the device (Fcnt = 0), sends the last stored message to the gateway from captured data where is device with the highest counter value. The attack replicates the last message and sends it over the gateway to the server.Server assumes that it receives a higher message than the attacker has sent and awaits this message. Sensor reboot and sending messages until it reaches the counter of the attacking message. Sensor is dosing for this time. This attack can be harmful for ABP activated end devices.

Attack detection is performed by saving data to the list (DeviceList). Information is retrieved from incoming physical payload (PHYPayload) by parsing and revers octets. Each row in DeviceList contains device has a counter (FCnt) of received message and information about restart the device (RESTART). If the device is restarted, RESTART value is set to 1. The device address (DevAddr) is used as the row index. An attacker is recognized if his last message is the same as the message after restarting the device. Identification of the attacker is based on same couture (FCnt).

![alt text](https://github.com/CESNET/NEMEA-SIoT/blob/master/lora-replay/al_replay_attack.png)

In the first part of the algorithm depicted in the diagram in figure, a packet is inserted into the input, which is then the input parameter for the (Packet decoder). The decoder will parse and reverse octets to retrieve packet information. The information we obtained from the decoder is then the input parameters for the next algorithm processing. The GetDeviceObject (DevAddr) command obtains an object from the worksheet list if the object is not found to return NULL. Under the following condition, the object is checked whether it is empty, ie NULL. If the object was not found and the condition was FALSE, the next step is to define the variables for the new object and insert object by using the Insert statement to DeviceList. This will bring the algorithm to an end. Otherwise, when the object is not empty (TRUE), it follows the compound condition (RESTART == 1) && (LAST_FCNT == FCnt) && (FCnt! = 0), if this condition is true, it is probably the attacker ATTACK DETECTION. Subsequently, the reset command is called RESTART = 0 and also if compound conditions are not followed by the exit command. The algorithm continues to check the size of the current packet message (FCnt> 0), if the FCnt is greater than zero, then the values of the found object (Device) are set where the current values for LAST_FCNT = FCnt. Otherwise, the original values of the object remain. The last check is to determine whether this is a restart using the condition FCnt == 0. If this is a restart, which means that the message counter has been reset and set to 0, the RESTART indicator will be set to state 1, using RESTART = 1. This is the end of the algorithm, it is called again when the packet arrives from the network again.

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
