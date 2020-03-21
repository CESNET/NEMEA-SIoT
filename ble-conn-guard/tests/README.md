# Tests for BLE Connection Guard
All test input datasets contain UniRec records which simulate BLE connections detected by [ble-conn-detector](https://github.com/CESNET/NEMEA-SIoT/tree/master/ble-conn-detector). All test scenarios use the same configuration file [ble-conn-guard.ini](ble-conn-guard.ini).

## Connection detection turned off for unknown devices
Use Case: This test verifies that when the configuration is set to "never" report discovered connections, the connections are really ignored even if ble-conn-detector announces them.

Configuration section: general

Input: Ignored_device.csv

### Input
Input contains information about the connection of device 00:25:96:12:34:56 which is not defined in the configuration and thus general rules apply.
 
### Expected output
As there is only one connection from unknown source that should never be reported, output should contain only field names.

## Report all connections of a device
Use Case: This test verifies that all connection af the device are reported if set to report "always".

Configuration section: general, c4:85:08:2b:ef:bb

Input: Monitored_device.csv

### Input
Input contains information about two connections of device c4:85:08:2b:ef:bb and one connection of the device 00:25:96:12:34:56.
 
### Expected output
Output should contain both connections of device c4:85:08:2b:ef:bb as the configuration specifies all connections shall be reported, but device 00:25:96:12:34:56 should be missing.

## Check of connections in specified timespan
Use Case: This test verifies that device can be set in configuration to be monitored in given time span and all connections to this device within this time shall be reported.

Configuration section: 6d:73:5a:f3:7b:98

Input: Timed_monitoring.csv

### Input
Input contains information about two connections of device 6d:73:5a:f3:7b:98 at 15:24:54 and 22:25:02.
 
### Expected output
Output shall contain only the second connection, because the monitoring is set between 18:00 and 3:30.
