# Wsn-anomaly detector module

## Description
This NEMEA modules detect anomalies in data streams using time series. 

This module is optimised for IoT networks and IoT use cases. More detailed description you can see in the [description](../description) directory. Test scenarios and configuration examples are in the [test](../test) directory.

## Interfaces
 - Input: 1
 - Output: 1 (alerts)

## Parameters
### Module specific parameters
  - `-c`    Cofiguration file with detection rules

### Common TRAP parameters
- `-h [trap,1]`      Print help message for this module / for libtrap specific parameters.
- `-i IFC_SPEC`      Specification of interface types and their parameters.
- `-v`               Be verbose.
- `-vv`              Be more verbose.
- `-vvv`             Be even more verbose.

