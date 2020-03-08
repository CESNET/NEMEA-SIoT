# Tests for LoRaWAN Airtime regulations module
All test input datasets contain UniRec records which simulates traffic statistics traffic captured by lora-collector.

## LoRaWAN Airtime regulations - Duty cycle 10%
Use Case: This test verifies the correct detection where breach of the regulated conditions. The duty cycle of radio devices is often regulated by government. For data simulation we used real captured data from the traffic of the campus gateway.

Test: For 10% duty cycle means the signal is on 10% of the time but off 90% of the time.

Configuration parameters: -d 10 (other parameters is default)

Input Traffic: duty_cycle_10.csv

### Input
The input file contains 105 messages capture from real gateway traffic.
    
### Expected output
The output will show 8 messages where the 2147491366, 2149974301 and 3232366755 devices has exceeded transmission time.
