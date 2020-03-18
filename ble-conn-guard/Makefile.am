bin_PROGRAMS=siot-ble-conn-guard
siot_ble_conn_guard_SOURCES=BLEConnGuard.cpp Configuration.h INIReader.cpp INIReader.h ini.c ini.h fields.c fields.h
siot_ble_conn_guard_LDADD=-ltrap -lunirec
siot_ble_conn_guard_LDFLAGS=-Wno-write-strings
siot_ble_conn_guard_CXXFLAGS=--std=c++11 -Wno-write-strings

include ../aminclude.am
