include(CMakeParseArguments)
include(ValidationHelpers)
include(FetchContent)

#===============================================================================
# alkos_setup_gtest
#===============================================================================
# Fetches GoogleTest if not already available.
function(alkos_setup_gtest)
    if(NOT TARGET GTest::gtest_main)
        message(STATUS "[HostTests] Fetching GoogleTest...")
        FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/refs/heads/main.zip
        )
        # Prevent GTest from overriding our compiler flags (like -Werror)
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(googletest)
        include(GoogleTest)
    endif()
endfunction()

#===============================================================================
# alkos_add_host_test_suite
#===============================================================================
# Creates ONE executable for all .cpp files in a directory, links GTest,
# and registers it with CTest.
#
# Parameters:
#   TARGET_NAME       Name of the suite (e.g., "containers").
#   LIBS_UNDER_TEST   Targets to test (e.g., alkos.libcontainers.interface).
#   DIRECTORY         Directory containing .cpp files.
#
function(alkos_add_host_test_suite)
    set(options)
    set(oneValueArgs TARGET_NAME DIRECTORY)
    set(multiValueArgs LIBS_UNDER_TEST)

    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    alkos_ensure_defined(VARS ARG_TARGET_NAME ARG_DIRECTORY ARG_LIBS_UNDER_TEST)

    file(GLOB TEST_SOURCES "${ARG_DIRECTORY}/*.cpp")

    if(NOT TEST_SOURCES)
        message(WARNING "[HostTests] No tests found in ${ARG_DIRECTORY}")
        return()
    endif()

    set(SUITE_EXE_NAME "test_suite_${ARG_TARGET_NAME}")

    add_executable(${SUITE_EXE_NAME} ${TEST_SOURCES})

    set_target_properties(${SUITE_EXE_NAME} PROPERTIES 
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
    )

    target_link_libraries(${SUITE_EXE_NAME} PRIVATE
        ${ARG_LIBS_UNDER_TEST}
        alkos.libcpp.interface # mock
        alkos.libc.interface   # mock
        GTest::gtest_main      
    )

    # Enable Sanitizers
    target_compile_options(${SUITE_EXE_NAME} PRIVATE -fsanitize=address -fsanitize=undefined -g)
    target_link_options(${SUITE_EXE_NAME} PRIVATE -fsanitize=address -fsanitize=undefined)

    # Register every TEST() inside the binary to CTest
    # This requires 'include(GoogleTest)' which is done in setup
    gtest_discover_tests(${SUITE_EXE_NAME}
        TEST_PREFIX "${ARG_TARGET_NAME}."
    )

    # Create a convenience make target (e.g. 'make check_containers')
    add_custom_target(check_${ARG_TARGET_NAME}
        COMMAND ${SUITE_EXE_NAME}
        COMMENT "Running ${ARG_TARGET_NAME} test suite..."
    )

    message(STATUS "[HostTests] Registered suite: ${SUITE_EXE_NAME}")
endfunction()
