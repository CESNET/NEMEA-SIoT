# Nemea Secured IoT 

This repository contains [Nemea system](https://github.com/CESNET/Nemea) modules for a threat detection in IoT networks. The modules and their functionality/purposes are:
 * [ble-adv-collector](ble-adv-collector) -- provide ble devices which are in proximity
 * [ble-pairing](ble-pairing) -- detect unexpected ble pairing
 * [hci-collector](hci-collector) -- exports ble packet date from hci interface
 * [lora-airtime](lora-airtime) -- detect unexpected frequency of lora messages
 * [lora-replay](lora-replay) -- detect replay attack in lora networks
 * [lora-distance](lora-distance) -- detect unexpected sensor location change
 * [wsn-anomaly](wsn-anomaly-testable) -- universal anomaly detector for wireless sensor networks

Also, there are [datasets](#iot-datasets) of IoT devices communication.

# Installation

You can install all available modules by the following steps: 

```
./bootstrap.sh
./configure
make
sudo make install
```

Also, you can install every module separatedly by running the above commands inside the appropriate directory.

# IoT Datasets
This folder includes IoT datasets for treat detection and ML trainig purposes. Extended flow data was created by [Joy tool](https://github.com/cisco/joy). Used initial parameters you can see in bash scripts inside [datasets](datasets) folder.

# Auto Tests
The script `AutoTest.py` serves for module unit testing.

### The course of testing

When launched with no arguments, it scans the repository and determines all directories containing modules. The criteria for this determination are: 
* A directory that contains a file named bootstrap.sh is considered to be a module written in C language.

When the modules are determined, the compiling phase follows. During this phase 
are executed commands `./bootstrap.sh`, `./configure` and `make` in each C module 
directory. If they succeed, the script checks the presence of an executable file 
with an expected name. The expected name of a module binary contains `siot-` prefix 
that is followed by the module name. Module name is considered to be identical with 
the name of the module directory. 

The last phase of the script is testing the module data processing. The script considers module to be suitable for testing if the directory tests is present in its module directory. The conventional structure of the tests directory is that it contains `.csv` and `.out` file pairs, where the `.csv` file contains test data that are proper for the module work demonstration. The `.out` file then contains an expected output of the module after processing the test data. Each testable module is tested as follows: 
 * A module is launched in its directory, and after one second it is checked if the module is still running. 
 * If so, the testing script injects the test data to the module input IFC using `NEMEA logreplay` and captures the module output using `NEMEA logger`. 
 * The output is then compared to the expected output, and if they are identical, the test succeeded. 
 * _Moreover, the testing script can also handle unexpected situations such as module segmentation fault while processing the data._
 
 ### Arguments
 - `-m M [M...]`      Specify modules to be tested.
 - `-n {T,C}`         Do not **C**ompile / **T**est.
 - `-L path`         Path to NEMEA Logger (if not sepcified considered as installed)
 - `-R path`         Path to NEMEA Logreplay (if not sepcified considered as installed)
 
 ##### Example usages
 > Compile and test all modules  <br/>
 >`python3 AutoTest.py`

 > Compile and test _wsn-anomaly_ and _lora-replay_  <br/>
 >`python3 AutoTest.py -m wsn-anomaly lora-replay`
 
 > Just test _wsn-anomaly_ <br/>
 >`python3 AutoTest.py -m wsn-anomaly -n C`
 
 > Do nothing <br/>
 >`python3 AutoTest.py -n C -n T`
 
 # Integration Tests
 The script `IntegrationTest.py` serves for integration testing on Turris Omnia.
 
 ### The course of testing
 
 The work of the integration test script and auto test script is significantly alike. The main
 difference is that the integration test launches all modules at once. For this purpose, the
 NEMEA SupervisorL is used. For proper testing it is required to have the SupervisorL installed and turned off.
 
 The module first create a backup of potentially present SupervisorL configuration file and replaces it with the one for testing.
 This backup is restored when the script shuts down.
 
 Modules are tested same way as by AutoTest. The testing script injects the test data to the module input IFC using `NEMEA logreplay` 
 and captures the module output using `NEMEA logger`. The output is then compared to the expected output, and if they are identical, the test succeeded.
  
  ### Arguments
  - `-m M [M...]`      Specify modules to be tested.
  - `-L path`         Path to NEMEA Logger (if not sepcified considered as installed)
  - `-R path`         Path to NEMEA Logreplay (if not sepcified considered as installed)
  - `-p`              Pause the script before and after the test so that it is possible to verify wether modules are correctly running
  
  ##### Example usages
  > Test all modules  <br/>
  >`python3 IntegrationTest.py`
 
  > Test _ble-pairing_ and _lora-replay_  <br/>
  >`python3 AutoTest.py -m ble-pairing lora-replay`
