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

add_library(target.properties INTERFACE) # Populated by the toolchain file

################################################################################
#                                   Conf file                                  #
################################################################################

set(CMAKE_CONF_FILE "${CMAKE_SOURCE_DIR}/../config/conf.generated.cmake")

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

message(STATUS "Building for architecture: ${ARCH}")

message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")
message(STATUS "Using build type: ${CMAKE_BUILD_TYPE}")

message(STATUS "Using C compiler: ${CMAKE_C_COMPILER}")
message(STATUS "Using C++ compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Using AR: ${CMAKE_AR}")
message(STATUS "Using Ranlib: ${CMAKE_RANLIB}")
message(STATUS "Using Linker: ${CMAKE_LINKER}")
message(STATUS "Using gdb: ${CMAKE_GDB}")

message(STATUS "Using C flags: ${CMAKE_C_FLAGS}")
message(STATUS "Using C++ flags: ${CMAKE_CXX_FLAGS}")
message(STATUS "Using ASM_NASM flags: ${CMAKE_ASM_NASM_FLAGS}")

################################################################################
#                                   Sysroot                                    #
################################################################################

set(SYSROOT ${CMAKE_BINARY_DIR}/sysroot)
set(CMAKE_SYSROOT ${SYSROOT})

# Create the sysroot directories
file(MAKE_DIRECTORY ${CMAKE_SYSROOT}/usr/include)
file(MAKE_DIRECTORY ${CMAKE_SYSROOT}/usr/lib)
file(MAKE_DIRECTORY ${CMAKE_SYSROOT}/boot)
file(MAKE_DIRECTORY ${CMAKE_SYSROOT}/boot/grub)

################################################################################
#                              Load subdirectories                             #
################################################################################

set(SYSTEM_LIB_TYPE k)

# Add the subdirectories
add_subdirectory(libc)
add_subdirectory(generated)
add_subdirectory(kernel)

################################################################################
#                                   Aliases                                    #
################################################################################
alkos_ensure_property_defined(ARCH "Architecture must be defined in the toolchain file!")

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
