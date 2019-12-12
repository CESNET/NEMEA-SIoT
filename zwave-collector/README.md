# README
# Z-Wave Collector

## Description
This tool is intended to parse Z-Wave frames received from SDR Sniffer, convert
some values (bit fields, frame types, command classes, system cc commands, routing hops, etc.)
to human readable form and export them through output NEMEA interface. Corrupted
and unsupported (unrecognized) frames are dropped.

## Input Unirec Interface
	time    TIMESTAMP
	uint8   CHANNEL   - 1 (868.42MHz, 9.6kB/s), 2 (868.42MHz, 40kB/s) or 3 (869.85MHz, 100kB/s)
	bytes   FRAME     - variable length frame

## Output Unirec Interface
Transport header fields:

	time    TIMESTAMP
	uint64  DEV_ADDR      - source node id 
	uint8   CHANNEL       - 1 (868.42MHz, 9.6kB/s), 2 (868.42MHz, 40kB/s) or 3 (869.85MHz, 100kB/s)
	uint32  HOME_ID       - home id of z-wave network
	uint8   DST_ID        - destination node id
	uint8   SIZE          - size of frame in bytes
	string  TYPE          - "Singlecast" / "Broadcast" / "Ack"
	uint8   ACK_REQ       - frame (request) requires ack
	uint8   SEQ_NUM       - sequence number of frame
	uint8   ROUTED        - frame is routed (1 / 0) - has network header

Network header fields (empty or 0 if header is not present):

	string  ROUTED_TYPE   - "Request" / "Ack" / "Nack"
	uint8   SRC_HOP       - current source node id in route
	uint8   DST_HOP       - current destination node id in route
	uint8   FAILED_HOP    - failed node id in route (if route failed)
	bytes   HOPS          - route hops node ids

Payload fields (empty or 0 for ACK frames):
	
	bytes   PAYLOAD       - payload in bytes
	string  CMD_CLASS_STR - Command Class coverted to string
	string  CMD_STR       - System CC Command coverted to string (if present)

## Parameters
### Common TRAP parameters
- `-h [trap,1]`      Print help message for this module / for libtrap specific parameters.
- `-i IFC_SPEC`      Specification of interface types and their parameters.
- `-v`               Be verbose.
- `-vv`              Be more verbose.
- `-vvv`             Be even more verbose.
