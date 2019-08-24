bin_PROGRAMS=siot-ble-adv-collector
siot_ble_adv_collector_SOURCES=BLEAdvCollector.cpp BLEAdvScanner.cpp BLEAdvScanner.h fields.c fields.h
siot_ble_adv_collector_LDADD=-ltrap -lunirec -lbluetooth
siot_ble_adv_collector_LDFLAGS=-Wno-write-strings
siot_ble_adv_collector_CXXFLAGS=--std=c++11 -Wno-write-strings

include ./aminclude.am
