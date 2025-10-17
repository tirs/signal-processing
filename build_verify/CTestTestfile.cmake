# CMake generated Testfile for 
# Source directory: C:/Users/simba/Desktop/Signal-Processing
# Build directory: C:/Users/simba/Desktop/Signal-Processing/build_verify
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(UnitTests "C:/Users/simba/Desktop/Signal-Processing/build_verify/Debug/unit_tests.exe")
  set_tests_properties(UnitTests PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/simba/Desktop/Signal-Processing/CMakeLists.txt;32;add_test;C:/Users/simba/Desktop/Signal-Processing/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(UnitTests "C:/Users/simba/Desktop/Signal-Processing/build_verify/Release/unit_tests.exe")
  set_tests_properties(UnitTests PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/simba/Desktop/Signal-Processing/CMakeLists.txt;32;add_test;C:/Users/simba/Desktop/Signal-Processing/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test(UnitTests "C:/Users/simba/Desktop/Signal-Processing/build_verify/MinSizeRel/unit_tests.exe")
  set_tests_properties(UnitTests PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/simba/Desktop/Signal-Processing/CMakeLists.txt;32;add_test;C:/Users/simba/Desktop/Signal-Processing/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test(UnitTests "C:/Users/simba/Desktop/Signal-Processing/build_verify/RelWithDebInfo/unit_tests.exe")
  set_tests_properties(UnitTests PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/simba/Desktop/Signal-Processing/CMakeLists.txt;32;add_test;C:/Users/simba/Desktop/Signal-Processing/CMakeLists.txt;0;")
else()
  add_test(UnitTests NOT_AVAILABLE)
endif()
