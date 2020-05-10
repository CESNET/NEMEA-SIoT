# README
# Z-Wave SDR Sniffer

## Description
This tool is intended to sniff Z-Wave traffic using Software Defined Radio (SDR)
and export frames one by one (including corrupted frames for further processing) with
timestamp and channel (on which they were captured) using NEMEA output interface.
 
The Sniffer uses 2 RTL-SDR dongles for receiving simultaneously on both Z-Wave
EU frequencies 868.40 MHz and 869.85 MHz.
Decoding the Z-Wave frames (ITU G.9959) is realized using rtl_zwave: https://github.com/andersesbensen/rtl-zwave.

Installed rtl-sdr package is required.
- Debian based systems `apt install rtl-sdr`
- OpenWrt `opkg install rtl-sdr`

## Output Unirec Interface
	time    TIMESTAMP
	uint8   CHANNEL   - 1 (868.40MHz, 9.6kB/s), 2 (868.40MHz, 40kB/s) or 3 (869.85MHz, 100kB/s)
	bytes   FRAME     - variable length frame

## Parameters
### Module specific parameters
- `-g  --gain <uint8>`  Gain of rtl-sdr dongles (default 30).

### Common TRAP parameters
- `-h [trap,1]`      Print help message for this module / for libtrap specific parameters.
- `-i IFC_SPEC`      Specification of interface types and their parameters.
- `-v`               Be verbose.
- `-vv`              Be more verbose.
- `-vvv`             Be even more verbose.
