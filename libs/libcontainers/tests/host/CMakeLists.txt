alkos_add_host_test_suite(
    TARGET_NAME     containers
    LIBS_UNDER_TEST alkos.libcontainers.interface
    DIRECTORY       ${CMAKE_CURRENT_SOURCE_DIR}
)
