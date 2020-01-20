bin_PROGRAMS=siot-zwave-detector
siot_zwave_detector_SOURCES=ZWaveDetector.cpp ZWaveDetector.h ../zwave/ZWave.cpp ../zwave/ZWave.h fields.c fields.h
siot_zwave_detector_LDADD=-ltrap -lunirec
siot_zwave_detector_LDFLAGS=-Wno-write-strings -pthread
siot_zwave_detector_CXXFLAGS=--std=c++11 -Wno-write-strings
