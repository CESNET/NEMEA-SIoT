cmake_minimum_required(VERSION 3.6)
project(zwave-sdr-collector)

set(CMAKE_CXX_STANDARD 11)

find_package(Threads)

add_executable(
    zwave-sdr-collector
    ZWaveSDRCollector.cpp
    ZWaveRtlSdr.cpp
    ../zwave/ZWave.cpp
    fields.c)

target_link_libraries(zwave-sdr-collector trap unirec ${CMAKE_THREAD_LIBS_INIT})
