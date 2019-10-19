# WSN-Anomaly Examples
## Overview
- [configurations](https://github.com/CESNET/NEMEA-SIoT/tree/wsn-tests/wsn-anomaly/details/manual-demo-tests/configurations) contains commented examples of configuration files
- [recordings](https://github.com/CESNET/NEMEA-SIoT/tree/wsn-tests/wsn-anomaly/details/manual-demo-tests/recordings) contains captured UniRec traffic from BeeeOn gateway
- [demo-playground](https://github.com/CESNET/NEMEA-SIoT/tree/wsn-tests/wsn-anomaly/details/manual-demo-tests/demo-playground) contains captured UniRec traffic from BeeeOn gateway with prepared configuration file and expected output for each input file

## Description
### Recordings
- Contain UniRec traffic recorded by NEMEA module logger
- Almost every recordig has two versions: normal and time
- Time version extends normal one by timestamp
- You can replay every recording by NEMEA module logreplay
- If logreplay finds a timestamp it replays traffic according to the timestamp. Otherwise, it will replay all data immediately.
- Tests for periodic check were not possible to record because actual time is required in these check. In the appropriate files (onExport-z-wave-periodic-time.log, ble-periodic-time.log) are hints for generating the traffic.

### Recorded Anomaly Types
- ble-dos: contain different increase ratio for BLE network
- z-wave-dos-driver: contain different increase ratio for Z-Wave network
- z-wave-dos-node: contain different increace ratio for Z-Wave sensor
- ble-periodic-time: hints for generation periodic BLE traffic
- onExport-z-wave-periodic: hints for generation periodic Z-Wave traffic
- onExport-dataLimit-virtual: contain increase/decrease of sensor values
- z-wave-connection: contain changes in communication channel quality stats
- z-wave-measurement: test traffic used for measurements profile items

### Configurations
- contains configuration examples
- each file contains a configuraion example with a single of more lines that demonstrate specific feature of wsn-anomaly detector
- files contain comments explaining configuration format and fields

### How To Run a Test (LEGACY Configuration)
1. Prepare detector configuration file, i.e. 
        SOFCount: 10;12;11;delta;-;-;-;profile(moving_average,moving_variance,moving_median,average,);moving_average(-,-,-,-,-,5,0.2,0,0,);moving_variance(-,-,-,-,-,5,0.2,0,0,);average(-,-,-,-,-,5,0.2,0,0,);moving_median(-,-,-,-,-,5,0.2,0,0,);export(-,);

2. Run logger for alert output interface
        logger -t -T -i u:alert

3. Optionaly you can run more logger instances for periodic export feature

4. Run detector (you can remove or increase the vebose mode)
        ./siot-wsn-anomaly -i "u:zwave,u:alert" -c config.txt -l -v

5. Run logreplay for recorded traffic
         logreplay -i "u:zwave" -f z-wave-connection.log -n

### How To Run a Test (INI Configuration)
1. Prepare detector configuration file (check configurations/ini directory)
    
2. Run logger for alert output interface
        logger -t -T -i u:alert

3. Optionaly you can run more logger instances for periodic export feature

4. Run detector (you can remove or increase the vebose mode)
        ./siot-wsn-anomaly -i "u:zwave,u:alert" -c config.ini -v

5. Run logreplay for recorded traffic
         logreplay -i "u:zwave" -f z-wave-connection.log -n

Note: Recommended format of configuration is ini

### Demo Playground
For all input traffic files is prepared one configuration file. The same configuration is available in both formats INI (config.ini) and LEGACY (config.txt). For each input traffic file (suffix <name>.csv) there is file with expected output from wsn-anomly detector module (suffix <name>.out).
