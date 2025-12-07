cmake_minimum_required(VERSION 3.30)
project(AlkOS_Host_Tests LANGUAGES C CXX)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")
set(ALKOS_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")

include(SourceHelpers)
include(HostTestHelpers)

# Setup GTest
alkos_setup_gtest()
enable_testing()

# Setup Mocks
add_subdirectory(${ALKOS_ROOT_DIR}/tests/libcpp_mocks libcpp_mocks)

# Add Libs
add_subdirectory(${ALKOS_ROOT_DIR}/libs/libtemplate libtemplate) 
add_subdirectory(${ALKOS_ROOT_DIR}/libs/libcontainers libcontainers)

# Define Test Suites
add_subdirectory(${ALKOS_ROOT_DIR}/libs/libcontainers/tests/host tests_libcontainers)
