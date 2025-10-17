cmake_minimum_required(VERSION 3.10)
project(SatelliteSignalProcessing)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Main demonstration executable
add_executable(satellite_signal_demo main.cpp)
target_include_directories(satellite_signal_demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Benchmark executable
add_executable(benchmark_suite benchmark.cpp)
target_include_directories(benchmark_suite PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Unit tests executable
add_executable(unit_tests unit_tests.cpp)
target_include_directories(unit_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Compiler options
if(MSVC)
    target_compile_options(satellite_signal_demo PRIVATE /W4)
    target_compile_options(benchmark_suite PRIVATE /W4)
    target_compile_options(unit_tests PRIVATE /W4)
else()
    target_compile_options(satellite_signal_demo PRIVATE -Wall -Wextra)
    target_compile_options(benchmark_suite PRIVATE -Wall -Wextra)
    target_compile_options(unit_tests PRIVATE -Wall -Wextra)
endif()

# Enable testing
enable_testing()
add_test(NAME UnitTests COMMAND unit_tests)