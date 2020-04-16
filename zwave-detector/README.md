# README
# Z-Wave Detector

## Description
This module is intended to detect network scanning and attacks on routing in Z-Wave
network.
 
The Detector receives captured frames from SDR Sniffer through the first input
interface and find unexpected behavior in them. It also receives events from
the SIoT gateway through second input interface. These events are needed to know
Z-Wave controller initialization and pairing of sensory devices with timestamp.

To avoid duplicate alerts, individual attacks are aggregated and reported at
a specified time interval. Time window is also needed to synchonize between new
paired devices from events a attacks found in frames.

### Network Scanning
Attacker can use the following messages to obtain information from devices
in the network. Normally, these messages only occur during pairing, making it
easy to detect network scanning.

- Device Manufacturer and Device Type Request
- Request for Device Software Version
- Request for Supported Command Classes
- Request for Basic Operational Status
- Request for Configuration Settings

### Attacks on Routing
The common denominator of these attacks is that an attacker tries to attack
a routing in the network to break it and cause communication problems,
or he can act as a MITM node, so he can create a Black Hole and drop network
communication to shut down a node in the network or impersonate it without
disclosure. The attacker uses an node id that is not paired with the controller,
so using SDR Sniffer and capturing network traffic together with knowing the paired
devices directly from the Nemea Collector on the gateway is possible to catch him.
- Modification of the Neighbor List: attacker is trying to intrude himself to the NL
of some node, so uknown node id in the associated messages identifies him.
- Modification of the SR Cache: attacker is trying to intrude himself to SR Cache
or BackBone Cache of some node, so uknown node id in the associated messages identifies him.
- MITM Node - attacker act as an uknown MITM node in the SR of routed messages, making it
easily detectable.

## 1. Input Unirec Interface - frames
	time    TIMESTAMP - Time of frame capturing
	uint8   CHANNEL   - 1 (868.42MHz, 9.6kB/s), 2 (868.42MHz, 40kB/s) or 3 (869.85MHz, 100kB/s)
	bytes   FRAME     - Variable length frame

## 2. Input Unirec Interface - events
	time    TIMESTAMP  - Time of event
	double  EVENT_TYPE - Driver Ready (18), Node Added (6), ...
	double  HOME_ID    - Home Id of the Z-Wave Network
	double  NODE_ID    - Node Id the event is associated with

## Output Unirec Interface
	time    TIMESTAMP         - Attack start time
	uint64  INCIDENT_DEV_ADDR - Attacked node or MITM node in case of MITM attack
	uint32  ALERT_CODE        - Network Scanning (1)
	                          - Modification of the Neighbor List (2)
	                          - Modification of the SR Cache (3)
	                          - MITM Node (4)
	string  CAPTION           - Network Scanning: command classes used in scanning
	                          - Modification of the Neighbor List: unknown attacker's node ids
	                          - Modification of the SR Cache: unknown attacker's node ids
	                          - MITM Node: attacked routes

## Parameters
### Module specific parameters
- `-a  --alert_interval <uint8>`  Interval to report alerts in seconds (default 10s).
- `-s  --sync_time_window <uint8>`  Time window for synchronization events and frames (default 3s).
- `-p  --pairing_time_window <uint8>`  Time window to not report scan attack alert during pairing (default 5s).
- `-n  --network <string>`  Home ID of the Z-Wave network (not needed, if gateway is capable to send events).

### Common TRAP parameters
- `-h [trap,1]`      Print help message for this module / for libtrap specific parameters.
- `-i IFC_SPEC`      Specification of interface types and their parameters.
- `-v`               Be verbose.
- `-vv`              Be more verbose.
- `-vvv`             Be even more verbose.
