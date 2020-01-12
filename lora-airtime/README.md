# README
# LoRaWAN Detection - Airtime regulations

## Description
This detector is intended for LoRaWAN monitoring of airtime for individual sensors. The detector can decode payload based on Network Session Key and Application Session Key.

The inputs of this detector are fields containing size payload SIZE, spreading factor SF, bandwidth BAD_WIDTH, code rate CODE_RATE, time stamp record TIMESTAMP and payload from message PHY_PAYLOAD. These values are captured from LoRaWAN packet.

## Input Unirec Interface
	time    TIMESTAMP
	uint64  DEV_ADDR       - device address
        string  PHY_PAYLOAD    - payload from message
        uint32  SIZE
        uint32  SF
        uint32  BAD_WIDTH
        uint32  CODE_RATE

## Output Unirec Interface
	time    TIMESTAMP
	macaddr INCIDENT_DEV_ADDR
	uint32  ALERT_CODE
	string  CAPTION
	string  FCNT                - message counter
        uint64  AIR_TIME
        string  PHY_PAYLOAD         - payload from message
  
## Parameters
### Module specific parameters
- `-e  --header <int>`         Defines explicit header 1/0 (true/false), default value 1 (true).
- `-r  --data-rate <int>`      Low data rate optimization 1/0 (true/false)
- `-p  --preamble <int>`       Preamble symbol is defined for all regions in LoRaWAN 1.0 standard is 8, this is a default value.
- `-d  --dutycycle <double>`   Defines time between packet subsequence starts, default value dutycycle is 0.10. Dutycycle is expressed as a percentage.


### Common TRAP parameters
- `-h [trap,1]`      Print help message for this module / for libtrap specific parameters.
- `-i IFC_SPEC`      Specification of interface types and their parameters.
- `-v`               Be verbose.
- `-vv`              Be more verbose.
- `-vvv`             Be even more verbose.
