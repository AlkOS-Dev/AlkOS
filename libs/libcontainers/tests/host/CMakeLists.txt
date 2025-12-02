cmake_minimum_required(VERSION 3.30)

project(alkos_host_tests_libcontainers)

set(CMAKE_CXX_STANDARD 23)

#===============================================================================
# Define MOCK Interfaces (Dependency Injection)
#===============================================================================

add_subdirectory(
    "${CMAKE_SOURCE_DIR}/../../../../tests/libcpp_mocks"
    "${CMAKE_CURRENT_BINARY_DIR}/libmocks"
)

#===============================================================================
# Add REAL Libraries
#===============================================================================

add_subdirectory(
    "${CMAKE_SOURCE_DIR}/../../../libtemplate"
    "${CMAKE_CURRENT_BINARY_DIR}/libtemplate"
)

add_subdirectory(
    "${CMAKE_SOURCE_DIR}/../../../libcontainers"
    "${CMAKE_CURRENT_BINARY_DIR}/libcontainers"
)


#===============================================================================
# GTest
#===============================================================================

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

#===============================================================================
# Exe & Linking & Flags
#===============================================================================

add_executable(container_tests
    test_circular_buffer.cpp
)

target_link_libraries(container_tests PRIVATE
    alkos.libcontainers.interface
    GTest::gtest_main
    pthread 
)

target_compile_options(container_tests PRIVATE -Wall -Wextra -g -fsanitize=address -fsanitize=undefined)
target_link_options(container_tests PRIVATE -fsanitize=address -fsanitize=undefined)
