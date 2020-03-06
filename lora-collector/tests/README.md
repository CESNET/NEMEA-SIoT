# Tests for LoRaWAN Collector
All test input datasets contain UniRec records which simulates traffic statistics traffic captured by lora-collector.

## LoRaWAN packet parser - PKT Data 01 GW01
Use Case: This test verifies the correct decoding of information from the incoming payloud. For data simulation we used real captured data from the traffic of the campus gateway GW01.

Test: LoRaWAN parser test - it must contain the required correct values.

Input Traffic: pkt_data_01_GW01.csv

### Input
The first 17 messages have not contain AppEUI value in their payload.The following messages contain this value.
    
### Expected output
In the output shows an empty value AppEUI for the first 17 messages.