# Tests for BLE Connection Detector
All test input datasets contain UniRec records which simulates traffic statistics traffic captured by [ble-adv-collector](https://github.com/CESNET/NEMEA-SIoT/tree/master/ble-conn-detector). As a BLE device for this tests was chosen BeeWi SmartLight RGB Bulb, which was controlled by Android application.

## Toggle ON
Use Case: In this test the bulb is left 5 sec unconnected (for the models to initialise and capture some unconnected data), then the light is switched on and another 5 sec left on.

Input: 20191001_BeeWi_ON.csv

### Input
The input contains 128 advertising messages as they were captured. First one contain malformed data as reported by the collector to verify behaviour of the detector in case of such data. Then from the remaining 127 messages first 10 are used for model initialisation and every other message is compared to the model and the model is adjusted. Between messages 77 and 78 there was a connection to the device (the light was switched on).
    
### Expected output
In the output is expected one message containing the information about connection to the device, when the light was switched on.

## Toggle ON and then OFF
Use Case: In this test the bulb is left 5 sec unconnected (for the models to initialise and capture some unconnected data), then the light is switched on and another 5 sec left on. Then the light is switched off and left off for another 5 sec.

Input: 20191001_BeeWi_OFF.csv

### Input
The input contains 133 captured advertising messages with two connections - ON connection between messages 64 and 65 followed by the OFF connection between messages 103 and 104.
    
### Expected output
Two connections should be reported.

## Colour change
Use Case: In this test the bulb is switched into RGB mode and changed its colour. At first the detailed settings page is turned on in the application, which starts the BLE connection.Then is selected RGB mode, changed colour and detailed settings is closed (which closes the connection).

Input: 20191001_BeeWi_ColourChange.csv

### Input
The input contains 119 captured advertising messages with one connection between messages 67 and 68.
    
### Expected output
One connection, which is substantially longer than in previous tests, shall be reported.

# Passive
Use Case: This is a control test, how will the detector behave in case there are no connections at all. The device is turned on, but there is no connection and no command to it. This test should verify whether the model won't become too sensitive and start misbehaving.

Input: 20191001_BeeWi_Passive.csv

### Input
The input contains 114 captured advertising messages with no connection.
    
### Expected output
There should be no connection in the output of the module.

## Toggle OFF
Use Case: This test is complementary to the "Toggle ON" test. In the beginning the light is on and after 5 sec of wait, the light is turned off.

Input: 20191001_BeeWi_OFF.csv

### Input
The input contains 97 captured advertising messages with one connection between messages 61 and 62.
    
### Expected output
One connection shall be reported.

## Long connection
Use Case: This test verifies behaviour of the detector in case of untypically long connection. The detail settings page, which initiates connection, is opened and for about 40 seconds is kept open while changing settins. After that the detail settings page is closed and the device is left 5 seconds without connection.

Input: 20191001_BeeWi_LongConn.csv

### Input
The input contains 137 captured advertising messages with one connection between messages 71 and 72.
    
### Expected output
One connection with very long duration should be reported.
