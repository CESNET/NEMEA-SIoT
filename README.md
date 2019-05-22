# SIoT Gateway for TurrisOS

Run `./install.sh` to install all necessary packages to run SIoT Gateway.
See README in sub-directories for where you can find a guide on how to create a functional
secured gateway that consists of BeeeOn Gateway and SIoT detection modules on TurrisOS.
It is tested on Turrin Omnia with clean OpenWrt omnia 15.05 system that has disabled auto-updates.

## Run

To start BeeeOn Gateway run:
`/usr/bin/beeeon-gateway -c /usr/etc/beeeon/gateway/gateway-startup.ini`


To start NEMEA SIoT instance run:
`cp wsn-anomaly.conf /usr/etc/siot/wsn-anomaly.conf`
`cp supervirol.conf /etc/config/nemea-supervisor`
`/etc/init.d/nemea-supervisorl start`

ATTENTION! LoRa Distance and WSN Anomaly detectors are falling on a bus error.
Therefore, by default they are disabled. This can be changed in the `supervirol.conf` file.

### Interconnection with other system parts:
All LoRa detectors need LoRa Collector as a data source:
 * Launch the LoRa Collector on the VSB LoRa Gateway with output TCP IFC on port `3001`.
 * Check, and correct, the Gateway IP address in the supervisorl.conf. By default, it is set to `192.168.1.206`.

 To connect the Coliot system, launch multiple of Coliot Collector script.
 To connect to one detector output, one such instance is required. The input IFCs
 of these instances have to be set up to the IP address of a device the SIoT NEMEA system
 is deployed. Outputs are available at following ports:

  - `BLE Pairing` - `2011`
  - `WSN Anomaly` - `2001`
  - `LoRa Airtime` - `2021`
  - `LoRa Distance` - `2022`
  - `LoRa Replay` - `2023`
