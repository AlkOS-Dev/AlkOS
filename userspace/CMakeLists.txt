message(STATUS "Configuring Userspace programs...")

# Directory where we collect user binaries
set(USERSPACE_BIN_DIR "${CMAKE_CURRENT_BINARY_DIR}/bin")
alkos_add_to_bash_config("CONF_USERSPACE_BIN_DIR" "${USERSPACE_BIN_DIR}")

set(USER_LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/build/linker.ld")

# Create the Userspace Standard Library (libc)
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

    target_link_libraries(${PROG_NAME} PRIVATE c.userspace)

    add_custom_command(TARGET ${PROG_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROG_NAME}> ${USERSPACE_BIN_DIR}/${PROG_NAME}
        COMMENT "Installing ${PROG_NAME} to ${USERSPACE_BIN_DIR}"
    )
    
    add_dependencies(userspace_apps ${PROG_NAME})
endfunction()

# Register all user programs
alkos_add_subdirs(${CMAKE_CURRENT_SOURCE_DIR}/programs)
