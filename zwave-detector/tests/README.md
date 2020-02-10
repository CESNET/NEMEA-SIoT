# Tests for Z-Wave Detector
All test input datasets contains UniRec records which simulates Z-Wave attacks,
specifically network scanning and attacks on routing. Since the detector has 2
input interfaces each test has first input containing frames in file
(testname).0.csv and second input containing events from gateway in file
(testname).1.csv. Since the detector is time dependent
traffic captured with [Z-Wave SDR Sniffer](https://github.com/CESNET/NEMEA-SIoT/tree/zwave-modules-and-tests/zwave-sdr-sniffer)
and events from [NEMEA Collector](https://github.com/BeeeOn/gateway/blob/master/src/nemea/NemeaCollector.cpp),
has been minimalized and adapted to individual tests. Also not all routing attacks,
could be easily simulated and some attacks were generated as if they were present
on the network. All inputs of frames contain first invalid frame 'dummy' to manage
logreplay timing. Second frame is after 2 seconds to give time for proper
initialization.

All testcases use configuration in auto-test.json: alert reporting interval 5s,
synchronization time window 1s and pairing time window for scan attack 1s.

### Scan attack - Attacker requests all informations about node
Use case: Attacker impersonating controller and requests all valuable informations 
from node 2 (Device Manufacturer & Device Type, Soft. version, Supported CC...).
These messages appearing in communication in time of pairing, but that is not now,
so alert should be reported. Just one alert should be generated cuz alert interval
reporting window.

Test files: scan-attack-get-all.*
#### Input
Events: Controller initialization, Add node 2.
 
Frames: Multiple requests for valuable informations for node 2.
#### Expected output
Scan Attack alert with incident node id 2 and requested command classes.

### Scan attack - Node reports all informations
Use case: Node 2 reports valuable informations (Device Manufacturer & Device Type,
Soft. version...). These messages appearing in communication in time of pairing,
but that is not now, so possible attacker requested these informations and alert
should be reported. Just one alert should be generated cuz alert interval
reporting window.

Test files: scan-attack-report-all.*
#### Input
Events: Controller initialization, Add node 2.
 
Frames: Multiple reports of valuable informations from node 2.
#### Expected output
Scan Attack alert with incident node id 2 and reported command classes.

### Scan attack - Node reports information during pairing
Use case: Message requesting valuable informations of node 2 can mean scan attack,
but in pairing window interval is node 2 paired, so that informations are truly
requested from controller. No alert should be reported.

Test files: scan-attack-during-pairing-no-alert.*
#### Input
Events: Controller initialization, Add node 2 in pairing window.
 
Frames: Device Manufacturer & Device Type request from controller to node 2.
#### Expected output
Nothing


### Modification NL Attack - Attacker requests node with Do NL Test
Use case: Attacker impersonating controller and sends Do NL Test
message to node 2 with his node id 5 to break into his NL.

Test files: modification-nl-do-nl-test.*
#### Input
Events: Controller initialization, Add node 2.
 
Frames: Do NL Test message for node 2 containing unknown node id 5.
#### Expected output
Modification NL attack with incident node id 2 and uknown node 5.

### Modification NL Attack - Node sends NL Test to unknown node
Use case: Node sends NL Test to unknown node 5, after attacker sent him
Do NL Test with his node id 5. Controller can then request NL from node 2 and
attacker can break into Adjacency table of controler.

Test files: modification-nl-nl-test.*
#### Input
Events: Controller initialization, Add node 2.
 
Frames: NL Test message from node 2 to unknown node 5.
#### Expected output
Modification NL attack with incident node id 2 and uknown node 5.


### Modification NL Attack - Node sends NL Report with unknown node
Use case: Node sends NL Report message with unknown neighbor 5 to controller,
after requesting Do NL Test from attacker and doing NL Test. Some controllers
can accept this message and attacker can break Adjacency table of controller.

Test files: modification-nl-report-nl.*
#### Input
Events: Controller initialization, Add node 2.
 
Frames: NL Report message from node 2 to controller with attacker neighbor 5.
#### Expected output
Modification NL attack with incident node id 2 and uknown node 5.


### Modification NL Attack - Do not report false positive alert
Use case: Typical communication of requesting node 2 to test for neighbor 5
and reporting back to controller. Node 5 is paired so alert should not be reported.

Test files: modification-nl-node-paired-no-alert.*
#### Input
Events: Controller initialization, Add node 2,5.
 
Frames: Do NL Test, NL Test, NL Report messages with acknowledges between
controller and node 2 with tested and added node 5 to NL of node 2.
#### Expected output
Nothing

### Modification NL Attack - Possible attack is no reported after delayed pairing
Use case: Typical communication of requesting node 2 to test for neighbor 5
and reporting back to controller. In time of processing frames there is no known
node 5 so possible NL Attack should be stored and then reported. But node 5
is paired in alert interval so alert is dropped.

Test files: modification-nl-node-paired-delayed-no-alert.*
#### Input
Events: Controller initialization, Add node 2 and delayed add 5.
 
Frames: Do NL Test, NL Test, NL Report messages with acknowledges between
controller and node 2 with tested and added node 5 to NL of node 2.
#### Expected output
Nothing

### Modification SR Cache - SR Cache Assignment
Use case: Attacker impersonating controller and sends SR Cache Assignment
message to node 2 with fake route through his node id 5. Communication can be
then routed through attacker's MITM node.

Test files: modification-sr-cache-sr-cache-assignment.*
#### Input
Events: Controller initialization, Add node 2,3.

Frames: SR Cache assignment message for node 2 with route containing unknown node 5.
#### Expected output
Modification SR Cache alert with incident node id 2 and uknown node id 5.

### Modification SR Cache - BackBone Cache Assignment
Use case: Attacker impersonating controller and sends BackBone Cache Assignment
message to node 2 with fake route through his node id 5. Communication can be
then routed through attacker's MITM node.

Test files: modification-sr-cache-backbone-cache-assignment.*
#### Input
Events: Controller initialization, Add node 2,3.
 
Frames: BackBone Cache Assignment message for node 2 with route containing unknown node 5.
#### Expected output
Modification SR Cache alert with incident node id 2 and unknown node id 5.

### MITM Node
Use case: Unknown attacker's node with id 5 is found in routed communication
in route 3->2->5->1. There are six messages of this communication so just
one alert should be reported.

Test files: mitm.*
#### Input
Events: Controller initialization, Add Node 2,3.
 
Frames: Routed communication trough route 3->2->5->1.
#### Expected output
MITM alert with incident node id 5 and attacked route 3->2->5->1


### MITM Node - Do not report false positive alert
Use case: Verifies that false positive alert is not reported, if all nodes
of route are paired.

Test files: mitm-all-paired-no-alert.*
#### Input
Events: Controller initialization, Add Node 2,3,5
 
Frames: Routed communication 3->2->5->1
#### Expected output
Nothing
