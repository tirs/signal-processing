cmake_minimum_required(VERSION 3.10)
project(SatelliteSignalProcessing)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(satellite_signal_demo main.cpp)

target_include_directories(satellite_signal_demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(satellite_signal_demo PRIVATE /W4 /O2)
else()
    target_compile_options(satellite_signal_demo PRIVATE -Wall -Wextra -O2)
endif()