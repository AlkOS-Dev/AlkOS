cmake_minimum_required(VERSION 3.30)
project(AlkOS LANGUAGES C CXX ASM_NASM)
enable_language(ASM_NASM)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(PROJECT_NAME AlkOS)
set(PROJECT_VERSION "0.0")
set(PROJECT_AUTHOR "ALK Organisation")

################################################################################
#                                   Helpers                                    #
################################################################################

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(ValidationHelpers)
include(SourceHelpers)
include(RuntimeHelpers)

add_library(alkos.target.properties.interface INTERFACE) # Populated by the toolchain file

set(CMAKE_ROOT_DIR ${CMAKE_SOURCE_DIR})

################################################################################
#                                   Conf file                                  #
################################################################################

set(CMAKE_CONF_FILE "${CMAKE_ROOT_DIR}/config/conf.generated.cmake")

# Check if configuration was done
if (NOT EXISTS "${CMAKE_CONF_FILE}")
    message(FATAL_ERROR "Cmake configuration (conf.generated.cmake) file does not exists... Try running configure.bash script first!")
else ()
    message(STATUS "Detected configuration file. Updating variables...")
endif ()

include("${CMAKE_CONF_FILE}")

################################################################################
#                                   Toolchain                                  #
################################################################################

include(${CMAKE_TOOLCHAIN_FILE})
include(${CMAKE_FLAGS_FILE})

################################################################################
#                                   Messages                                   #
################################################################################

string(REPEAT "=" 80 separator)
message(STATUS "${separator}")
message(STATUS "AlkOS Build Configuration Summary")
message(STATUS "")
message(STATUS "  Architecture: ${ARCH}")
message(STATUS "  Build Type:   ${CMAKE_BUILD_TYPE}")
message(STATUS "  Toolchain:    ${CMAKE_TOOLCHAIN_FILE}")
message(STATUS "")
message(STATUS "  C Compiler:   ${CMAKE_C_COMPILER}")
message(STATUS "  C++ Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "  C Flags:      ${CMAKE_C_FLAGS}")
message(STATUS "  C++ Flags:    ${CMAKE_CXX_FLAGS}")
message(STATUS "  ASM (NASM) Flags: ${CMAKE_ASM_NASM_FLAGS}")
message(STATUS "")
message(STATUS "  Linker:       ${CMAKE_LINKER}")
message(STATUS "  Archiver:     ${CMAKE_AR}")
message(STATUS "  Ranlib:       ${CMAKE_RANLIB}")
message(STATUS "  Debugger:     ${CMAKE_GDB}")
message(STATUS "${separator}")

################################################################################
#                                   Sysroot                                    #
################################################################################

set(SYSROOT ${CMAKE_BINARY_DIR}/sysroot)
set(CMAKE_SYSROOT ${SYSROOT})

file(MAKE_DIRECTORY ${CMAKE_SYSROOT}/usr/include)
file(MAKE_DIRECTORY ${CMAKE_SYSROOT}/usr/lib)
file(MAKE_DIRECTORY ${CMAKE_SYSROOT}/boot)
file(MAKE_DIRECTORY ${CMAKE_SYSROOT}/boot/grub)

################################################################################
#                              Load subdirectories                             #
################################################################################

set(SYSTEM_LIB_TYPE k)

add_subdirectory(libc)
add_subdirectory(generated)
add_subdirectory(kernel)

################################################################################
#                                   Aliases                                    #
################################################################################

alkos_ensure_defined(
    VARS ARCH
    MESSAGE "The ARCH variable must be set to the target architecture (e.g., x86_64, i686, arm64)."
)

message(STATUS "Creating build aliases for ARCH='${ARCH}'")

add_custom_target(iso
    DEPENDS iso-${ARCH}
    COMMENT "Alias for: make iso-${ARCH}"
)

add_custom_target(run
    DEPENDS run-${ARCH}
    COMMENT "Alias for: make run-${ARCH}"
)

add_custom_target(run-with-gdb
    DEPENDS run-with-gdb-${ARCH}
    COMMENT "Alias for: make run-with-gdb-${ARCH}"
)

add_custom_target(mount
    DEPENDS mount-${ARCH}
    COMMENT "Alias for: make mount-${ARCH}"
)

if (CMAKE_FEATURE_FLAG_RUN_TEST_MODE)
    add_custom_target(tests
        DEPENDS tests-${ARCH}
        COMMENT "Alias for: make tests-${ARCH}"
    )
endif()
