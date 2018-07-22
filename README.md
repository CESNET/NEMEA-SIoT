Nemea Secured IoT 
===============

This repository contains [Nemea system](https://github.com/CESNET/Nemea) modules for a threat detection in IoT networks. The modules and their functionality/purposes are:
 * [ble-pairing](ble-pairing) -- detect unexpected ble pairing
 * [hci-collector](hci-collector) -- exports ble packet date from hci interface
 * [lora-airtime](lora-airtime) -- detect unexpected frequency of lora messages
 * [lora-replay](lora-replay) -- detect replay attack in lora networks
 * [lora-distance](lora-distance) -- detect unexpected sensor location change
 * [wsn-anomaly](wsn-anomaly) -- universal anomaly detector for wireless sensor networks


Installation
============

You can install all available modules by the following steps: 

```
./bootstrap.sh
./configure
make
sudo make install
```

Also, you can install every module separatedly by running the above commands inside the appropriate directory.
