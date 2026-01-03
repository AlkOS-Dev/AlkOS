message(STATUS "Configuring Userspace...")

# Directory where we collect user binaries
set(USERSPACE_BIN_DIR "${CMAKE_CURRENT_BINARY_DIR}/bin")
alkos_add_to_bash_config("CONF_USERSPACE_BIN_DIR" "${USERSPACE_BIN_DIR}")

set(USER_LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/build/linker.ld")

################################################################################
# CRT0 - C Runtime Startup Object Library
################################################################################
#
# The CRT0 (C Runtime 0) contains the _start entry point for userspace programs.
# It MUST be compiled as a separate OBJECT library and linked FIRST to ensure
# that _start is placed at the program's base address.
#
add_library(crt0.userspace OBJECT
    "${CMAKE_SOURCE_DIR}/libs/libc/src/internal/crt0.cpp"
)
target_link_libraries(crt0.userspace PUBLIC alkos.libc.interface)
target_compile_options(crt0.userspace PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:
        -fno-builtin
        -fno-rtti
        -fno-exceptions
        -fno-stack-protector
        -mcmodel=small
        -mno-red-zone
    >
)

################################################################################
# Userspace Standard Library
################################################################################
add_library(c.userspace STATIC ${ALKOS_LIBC_SOURCES})
target_link_libraries(c.userspace PUBLIC alkos.libc.interface gcc)
target_compile_options(c.userspace PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:
        -fno-builtin
        -fno-rtti
        -fno-exceptions
        -fno-stack-protector
        -mcmodel=small
        -mno-red-zone
    >
)

add_library(cpp.userspace STATIC ${ALKOS_LIBCPP_SOURCES})
target_link_libraries(cpp.userspace PUBLIC alkos.libcpp.interface gcc)
target_compile_options(cpp.userspace PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:
        -fno-builtin
        -fno-rtti
        -fno-exceptions
        -fno-stack-protector
        -mcmodel=small
        -mno-red-zone
    >
)

# Create a target to collect all user programs
add_custom_target(userspace_apps COMMENT "Building all userspace applications")

function(alkos_register_userspace_app PROG_NAME SRC_FILES)
    add_executable(${PROG_NAME} ${SRC_FILES})
    
    # Temporarily hardcode flags for x86_64
    target_compile_options(${PROG_NAME} PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:
            -fno-builtin
            -fno-rtti
            -fno-exceptions
            -fno-stack-protector
            -mcmodel=small
            -mno-red-zone
        >
    )

    target_link_options(${PROG_NAME} PRIVATE
        -T "${USER_LINKER_SCRIPT}"
        -nostdlib
        -z max-page-size=0x1000
        -static
    )
    
    target_link_libraries(${PROG_NAME} PRIVATE
        crt0.userspace
        c.userspace
        cpp.userspace
    )

    add_custom_command(TARGET ${PROG_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROG_NAME}> ${USERSPACE_BIN_DIR}/${PROG_NAME}
        COMMENT "Installing ${PROG_NAME} to ${USERSPACE_BIN_DIR}"
    )
    
    add_dependencies(userspace_apps ${PROG_NAME})
endfunction()

#################################### Programs ##################################

# Register all user programs
message(STATUS "  -> Configuring Programs...")
alkos_add_subdirs(${CMAKE_CURRENT_SOURCE_DIR}/programs)
