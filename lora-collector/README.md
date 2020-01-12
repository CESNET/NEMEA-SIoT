# README
# LoRaWAN Collector

## Description


## Output Unirec Interface
        uint32  SIZE
        uint32  SF
        uint32  BAD_WIDTH
        uint32  CODE_RATE
        time    TIMESTAMP
        string  PHY_PAYLOAD          - payload from message
        double  RSSI                 - received signal strength sndication
        uint64  DEV_ADDR             - device address
        uint32  US_COUNT
        uint32  FRQ
        uint32  RF_CHAIN
        string  STATUS
        string  MOD
        double  SNR
        string  APP_EUI
        string  APP_NONCE
        string  DEV_EUI
        string  DEV_NONCE
        string  FCTRL
        string  FHDR
        string  FOPTS
        string  FPORT
        string  MHDR
        string  MIC
        string  NET_ID

## Parameters
### Common TRAP parameters
- `-h [trap,1]`      Print help message for this module / for libtrap specific parameters.
- `-i IFC_SPEC`      Specification of interface types and their parameters.
- `-v`               Be verbose.
- `-vv`              Be more verbose.
- `-vvv`             Be even more verbose.

