# README
# Z-Wave Statistics Creator

## Description
This module is intended to generate statistics captured from Z-Wave network
using SDR for WSN-Anomaly detector. These statistics provide a more global view
of the network.

Z-Wave Statistics Creator receives captured frames from Z-Wave SDR Sniffer through
input unirec interface and calculates statistics from traffic for whole network and
for every communicating node in the network. Statistics for network are sent through
first output unirec interface and statistics for nodes are sent with node
identification through second unirec interface.
## Input Unirec Interface
	time  TIMESTAMP - Time of frame capturing.
	uint8 CHANNEL   - 1 (868.42MHz, 9.6kB/s), 2 (868.42MHz, 40kB/s) or 3 (869.85MHz, 100kB/s)
	bytes FRAME     - Variable length frame.

## 1. Output Unirec Interface - Network
	time   TIMESTAMP       - Timestamp of generating statistics.
	uint64 DEV_ADDR        - Home Id.
	double CORRUPTED_C     - Total count of corrupted frames.
	double CORRUPTED_CH1_C - Count of corrupted frames on channel 1.
	double CORRUPTED_CH2_C - Count of corrupted frames on channel 2.
	double CORRUPTED_CH3_C - Count of corrupted frames on channel 3.
	double TOTAL_OK_C      - Total count of valid frames.
	double ROUTED_C        - Total count of routed frames.
	double ROUTED_ACK_C    - Count of routed ack frames.
	double ROUTED_NACK_C   - Count of routed nack frames.
	double ROUTED_APP_C    - Count of routed application frames.
	double SINGLECAST_C    - Count of singlecast frames.
	double ACK_C           - Count of ack frames.
	double MULTICAST_C     - Count of multicast frames.
	double BROADCAST_C     - Count of broadcast frames.
	double NET_MANAG_C     - Count of network management frames.

## 2. Output Unirec Interface - Nodes
	time   TIMESTAMP         - Timestamp of generating statistics.
	uint64 DEV_ADDR          - Node Id.
	double SRC_NACK_C        - Count of routed nack frames (node is src).
	double DST_NACK_C        - Count of routed nack frames (node is dst).
	double FAILED_HOP_NACK_C - Count of routed nack frames (node is failed hop).
	double SRC_TOTAL_C       - Total count of frames (node is src).
	double DST_TOTAL_C       - Total count of frames (node is dst).
	double SRC_SINGL_C       - Count of singlecast frames (node is src).
	double DST_SINGL_C       - Count of singlecast frames (node is dst).
	double SRC_ACK_C         - Count of ack frames (node is src).
	double DST_ACK_C         - Count of ack frames (node is dst).
	double SRC_MULTICAST_C   - Count of multicast frames (node is src).
	double SRC_BROADCAST_C   - Count of broadcast frames (node is src).
	double SRC_TOTAL_LM_T    - Last message timestamp (node is src).
	double DST_TOTAL_LM_T    - Last message timestamp (node is dst).
	double SRC_SINGL_LM_T    - Last singlecast message timestamp (node is src).
	double DST_SINGL_LM_T    - Last singlecast message timestamp (node is dst).

## Parameters
### Module specific parameters
- `-s  --stats-interval <uint8>`  Interval to generate statistics in seconds (default 10s).
- `-n  --network <string>`  Home ID of the Z-Wave network (to drop frames from other networks).
- `-t  --testing`  Testing mode (timestamp 0 in statistics).

### Common TRAP parameters
- `-h [trap,1]`      Print help message for this module / for libtrap specific parameters.
- `-i IFC_SPEC`      Specification of interface types and their parameters.
- `-v`               Be verbose.
- `-vv`              Be more verbose.
- `-vvv`             Be even more verbose.
