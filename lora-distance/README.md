# README
# LoRaWAN Detection - Change distance

## Description
This detector serves for detection changing distance between device and gateway. Detection is for fixed-position devices, if the attacker transfers the device, the RSSI (Received Signal Strength Indication) changes. This may vary depending on the environment, such as weather. Therefore, it is possible to set the variance for RSSI. Base RSSI value is defined by the first received message from device to detector.

![alt text](https://github.com/gre0071/lora_detector_change_distance/blob/master/change_distance.png)

The example shows the attacker's identification where the detector is set to 10% variance. This means that for -119 dBm is variance -11.9 dBm. The minimum value is -130.9 dBm and maximum -107.1 dBm. An attacker is therefore detected because it does not fall within the range.

## Input Unirec Interface
	time    TIMESTAMP
	uint64  DEV_ADDR       - device address
	double  RSSI           - received signal strength sndication
        string  PHY_PAYLOAD    - payload from message

## Output Unirec Interface
	time    TIMESTAMP
	macaddr INCIDENT_DEV_ADDR
	uint32  ALERT_CODE
	string  CAPTION
	double  RSSI                 - received signal strength sndication
	double  BASE_RSSI            - base RSSI value is defined by the first received message from device to detector
        double  VARIANCE

## Parameters
### Module specific parameters
- `-a  --variance <double>`         Define variance, default value 10% (0.1).

### Common TRAP parameters
- `-h [trap,1]`      Print help message for this module / for libtrap specific parameters.
- `-i IFC_SPEC`      Specification of interface types and their parameters.
- `-v`               Be verbose.
- `-vv`              Be more verbose.
- `-vvv`             Be even more verbose.
