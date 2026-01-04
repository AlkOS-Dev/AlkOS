message(STATUS "Configuring Userspace...")

# Directory where we collect user binaries
set(USERSPACE_BIN_DIR "${CMAKE_CURRENT_BINARY_DIR}/bin")
alkos_add_to_bash_config("CONF_USERSPACE_BIN_DIR" "${USERSPACE_BIN_DIR}")

################################ Linker Script #################################

set(USER_LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/build/${ARCH}/linker.ld")

################################################################################
# CRT Object Files for C++ Global Constructors
################################################################################

# Get the architecture-specific CRTI/CRTN objects from the libc configuration
get_property(CRT0_OBJ_PATH TARGET alkos.libc.config PROPERTY CRT0_OBJ)
get_property(CRTI_OBJ_PATH TARGET alkos.libc.config PROPERTY CRTI_OBJ)
get_property(CRTN_OBJ_PATH TARGET alkos.libc.config PROPERTY CRTN_OBJ)
alkos_ensure_defined(
    VARS CRT0_OBJ_PATH CRTI_OBJ_PATH CRTN_OBJ_PATH
    MESSAGE "CRT0_OBJ/CRTI_OBJ/CRTN_OBJ properties were not set by the libc architecture-specific CMake file."
)

# Find the compiler's CRT object files at configure-time.
execute_process(
    COMMAND ${CMAKE_C_COMPILER} -print-file-name=crtbegin.o
    OUTPUT_VARIABLE CRTBEGIN_OBJ_PATH
    ERROR_VARIABLE CRTBEGIN_ERROR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
    COMMAND ${CMAKE_C_COMPILER} -print-file-name=crtend.o
    OUTPUT_VARIABLE CRTEND_OBJ_PATH
    ERROR_VARIABLE CRTEND_ERROR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

alkos_ensure_defined(
    VARS CRTBEGIN_OBJ_PATH CRTEND_OBJ_PATH
    MESSAGE "Could not find compiler's crtbegin.o/crtend.o files."
)

# Configure the linker rule for userspace executables to properly support
# C++ global constructors by linking CRT objects in the correct order:
# crt0 -> crti -> crtbegin -> <program objects> -> crtend -> crtn
set(CMAKE_CXX_LINK_EXECUTABLE
    "${CMAKE_CXX_COMPILER} <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> ${CRT0_OBJ_PATH} ${CRTI_OBJ_PATH} ${CRTBEGIN_OBJ_PATH} <OBJECTS> ${CRTEND_OBJ_PATH} ${CRTN_OBJ_PATH} -o <TARGET> <LINK_LIBRARIES>"
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
        c.userspace
        cpp.userspace
    )

    add_custom_command(TARGET ${PROG_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROG_NAME}> ${USERSPACE_BIN_DIR}/${PROG_NAME}
        COMMENT "Installing ${PROG_NAME} to ${USERSPACE_BIN_DIR}"
    )

    add_dependencies(${PROG_NAME} alkos.libc.crt0 alkos.libc.crti alkos.libc.crtn)
    add_dependencies(userspace_apps ${PROG_NAME})
endfunction()

#################################### Programs ##################################

# Register all user programs
message(STATUS "  -> Configuring Programs...")
alkos_add_subdirs(${CMAKE_CURRENT_SOURCE_DIR}/programs)
