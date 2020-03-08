# Tests for LoRaWAN Replay attack ABP module
All test input datasets contain UniRec records which simulates traffic statistics traffic captured by lora-collector.

## LoRaWAN Replay attack ABP
Use Case: Test attack replicates the last message and sends it over the gateway to the server. Server assumes that it receives a higher message than the attacker has sent and awaits this message. The attacker is detected based on its behavior.

Test:This detector serves for detection replay attack in LoRaWAN infrastructure of ABP authentication method.

Input Traffic: replay_attack_ABP.csv

### Input
The input file contains 105 messages capture from real gateway traffic.Input includes 5 messages that simulate message replication message after device restart.
    
### Expected output
The output displays a message where the 2490654 device performed an ABP replay attack.
