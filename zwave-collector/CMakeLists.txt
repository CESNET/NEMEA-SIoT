cmake_minimum_required(VERSION 3.6)
project(zwave-collector)

set(CMAKE_CXX_STANDARD 11)

find_package(Threads)

add_executable(
    zwave-collector
    ZWaveCollector.cpp
    ../zwave/ZWave.cpp
    fields.c)

target_link_libraries(zwave-collector trap unirec)
