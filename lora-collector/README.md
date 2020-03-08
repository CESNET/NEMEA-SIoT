# LoRaWAN Collector

## Description
This tool is intended to parse LoRaWAN frames received from LoRaWAN Concetrator iC880A. Convert some values from payload (uint64 DEV_ADDR, string APP_EUI,string APP_NONCE,string DEV_EUI,string DEV_NONCE,string FCTRL,uint16 FCNT, uint8 MS_TYPE, string FHDR,string FOPTS,string FPORT,string MHDR,string MIC,string NET_ID) to human readable form and export them through output NEMEA interface. The output value STATUS determines whether the frame is valid. Tool implements Semtech to retrieve frames from the concentrator. This repository contains global and local gateway configuration files (global_conf.json, local_conf.json) for LoRaWAN. 

For connect to The Things Network global configuration here: https://github.com/TheThingsNetwork/gateway-conf


## Input Unirec Interface
This input is only active in the test mode. "-t"

    double  RSSI                - received signal strength sndication,average packet RSSI in dB
    double  SNR                 - average packet SNR, in dB (LoRa only)
    time    TIMESTAMP           - timestamp receive frame
    uint32  BAD_WIDTH           - modulation bandwidth (LoRa only)
    uint32  CODE_RATE           - error-correcting code of the packet (LoRa only)
    uint32  FRQ                 - central frequency of the IF chain
    uint32  SF                  - RX datarate of the packet (SF for LoRa)
    uint32  US_COUNT            - internal concentrator counter for timestamping, 1 microsecond resolution
    uint16  SIZE                - payload size in bytes
    uint8   MOD                 - modulation used by the packet
    uint8   RF_CHAIN            - through which RF chain the packet was received
    uint8   STATUS              - status of the received packet
    string  PHY_PAYLOAD         - payload from message

## Output Unirec Interface

    uint64  DEV_ADDR            - device address
    double  RSSI                - received signal strength sndication,average packet RSSI in dB
    double  SNR                 - average packet SNR, in dB (LoRa only)
    time    TIMESTAMP           - timestamp receive frame
    uint32  BAD_WIDTH           - modulation bandwidth (LoRa only)
    uint32  CODE_RATE           - error-correcting code of the packet (LoRa only)
    uint32  FRQ                 - central frequency of the IF chain
    uint32  SF                  - RX datarate of the packet (SF for LoRa)
    uint32  US_COUNT            - internal concentrator counter for timestamping, 1 microsecond resolution
    uint16  FCNT                - frame counter
    uint16  SIZE                - payload size in bytes
    uint8   MOD                 - modulation used by the packet
    uint8   MS_TYPE             - message type (Join Request, Join Accept, Unconfirmed Data Up, Unconfirmed Data Down, Confirmed Data Up, Confirmed Data Down)
    uint8   RF_CHAIN            - through which RF chain the packet was received
    uint8   STATUS              - status of the received packet
    string  APP_EUI             - the join procedure, identifies the end application
    string  APP_NONCE           - a nonce sent from network to device during a join response that allows the device to generate the session keys.
    string  DEV_EUI             - the join procedure requires the end-device to be personalized with the DevEUI information before it starts the join procedure. Unique per device, set by manufacturer
    string  DEV_NONCE           - DevNonce is a counter starting at 0 when the device is initially powered up and incremented with every Join-request 
    string  FCTRL               - frame control
    string  FHDR                - frame header
    string  FOPTS               - frame options
    string  FPORT               - optional port field
    string  MHDR                - MAC header
    string  MIC                 - message integrity code
    string  NET_ID              - server’s unique identifier NetID
    string  PHY_PAYLOAD         - payload from message

## Parameters

### Collector parameters
- `-d`               Be debugging mode.
- `-t`               Be test mode, packet can now be received from files.

### Common TRAP parameters
- `-h [trap,1]`      Print help message for this module / for libtrap specific parameters.
- `-i IFC_SPEC`      Specification of interface types and their parameters.
- `-v`               Be verbose.
- `-vv`              Be more verbose.
- `-vvv`             Be even more verbose.

