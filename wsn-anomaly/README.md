# WSN-Anomaly Time Series Detector Module

## Description
This NEMEA modules detect anomalies in data streams using time series. 

This module is optimised for IoT networks and IoT vulnerable use cases. More detailed description you can see in the [description](details/description) directory. Test scenarios, configuration examples and hints are in the [examples](details/manual-demo-tests) directory. The directory [tests](tests) is used for automatic integration tests to verify proprer functionality.

## Interfaces
 - Input: 1
 - Output: 1 (alerts) # Note: Using configuration file is possible to create output export interfaces

## Parameters
### Module specific parameters
  - `-c`    Cofiguration file with detection rules
  - `-l`    Legacy format of configuration file
  - `-I`    Do not terminate module on incoming termination message

### Common TRAP parameters
- `-h [trap,1]`      Print help message for this module / for libtrap specific parameters.
- `-i IFC_SPEC`      Specification of interface types and their parameters.
- `-v`               Be verbose.
- `-vv`              Be more verbose.
- `-vvv`             Be even more verbose.

