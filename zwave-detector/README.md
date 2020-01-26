# README
# Z-Wave Detector

## Description
This tool is intended to detect network scanning and attacks on routing in Z-Wave
network. In current version just network scanning is supported.
 
The Detector receives captured frames from SDR Sniffer through the first input
interface and find unexpected behavior in them. It also receives events from
the SIoT gateway through second input interface. These events are needed to know
Z-Wave controller initialization and pairing of sensory devices with timestamp.

Attacker can use the following messages to obtain information from devices
in the network. Normally, these messages only occur during pairing, making it
easy to detect network scanning.

- Device Manufacturer and Device Type Request
- Request for Device Software Version
- Request for Supported Command Classes
- Request for Basic Operational Status
- Request for Configuration Settings

## 1. Input Unirec Interface - frames
	time    TIMESTAMP
	uint8   CHANNEL   - 1 (868.42MHz, 9.6kB/s), 2 (868.42MHz, 40kB/s) or 3 (869.85MHz, 100kB/s)
	bytes   FRAME     - variable length frame

## 2. Input Unirec Interface - events
	time    TIMESTAMP
	double  EVENT_TYPE - 18 (driver ready), 6 (node added), 7 (node removed),...
	double  HOME_ID
	double  NODE_ID

## Output Unirec Interface
	time    TIMESTAMP
	uint64  INCIDENT_DEV_ADDR
	uint32  ALERT_CODE        - 1 (network scanning)
	string  CAPTION           - command classes used in scanning

## Parameters
### Module specific parameters
- `-a  --alert_interval <uint8>`  Time window for reporting alerts in seconds (default 10s).
- `-n  --network <string>`  Home ID of the Z-Wave network (not needed, if gateway is capable to send events).

### Common TRAP parameters
- `-h [trap,1]`      Print help message for this module / for libtrap specific parameters.
- `-i IFC_SPEC`      Specification of interface types and their parameters.
- `-v`               Be verbose.
- `-vv`              Be more verbose.
- `-vvv`             Be even more verbose.
