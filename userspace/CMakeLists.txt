message(STATUS "Configuring Userspace programs...")

# !!!
# NOTE: Hardcoded logic for x86_64 for now
# !!!

# Directory where we collect user binaries
set(USERSPACE_BIN_DIR "${CMAKE_BINARY_DIR}/userspace_bin")
file(MAKE_DIRECTORY ${USERSPACE_BIN_DIR})

set(USER_LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/build/linker.ld")
# Temporarily hardcode x86_64 stub for passing printf
# To be removed after proper libc is done
set(CRT0_SRC "${CMAKE_CURRENT_SOURCE_DIR}/crt0.nasm")

# Create a target to collect all user programs
add_custom_target(userspace_apps COMMENT "Building all userspace applications")

function(add_user_program PROG_NAME SRC_FILES)
    add_executable(${PROG_NAME} ${CRT0_SRC} ${SRC_FILES})
    
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

    add_custom_command(TARGET ${PROG_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROG_NAME}> ${USERSPACE_BIN_DIR}/${PROG_NAME}
        COMMENT "Installing ${PROG_NAME} to ${USERSPACE_BIN_DIR}"
    )
    
    add_dependencies(userspace_apps ${PROG_NAME})
endfunction()

add_user_program(hello programs/hello_world/main.cpp)
