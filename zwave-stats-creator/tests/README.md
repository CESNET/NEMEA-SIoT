# Tests for Z-Wave Statistics Creator
### Test of the correct generation of statistics
Test uses 10 selected frames captured with [Z-Wave SDR Sniffer](https://github.com/CESNET/NEMEA-SIoT/tree/zwave-modules-and-tests/zwave-sdr-sniffer),
to verify that statistics are generated correctly. These frames cover all individual
statistics for network and also node statistics and it was easy to calculate
that the statistics were produced correctly. These 10 frames were duplicated to
500 in single statistics report after 5 seconds.

Configuration in auto-test.json: testing mode (0 timestamp on output),
home_id e1453473 and statistics generating interval 5 seconds.

Test files: 50x10frame-after-5s.*

