message(STATUS "Configuring kernel...")

# This target acts as a central hub for configuration data (e.g., kernel modules)
# that needs to be communicated to the top-level CMakeLists.txt.
add_library(alkos.kernel.config INTERFACE)  
add_library(alkos.kernel.headers INTERFACE) # Kernel headers for exporting
add_library(alkos.kernel.deps INTERFACE)    # Target to link all dependencies to, to ensure
                                            # proper linking order

############################### Error Checking ###############################

alkos_ensure_defined(
    VARS SYSROOT ARCH
)

############################ Compile Definitions #############################

add_compile_definitions(__ALKOS_KERNEL__=1)
add_compile_definitions(__ALKOS_LIBK__=1) # For LIBK headers

########################### Setting Sysroot Boot #############################

file(MAKE_DIRECTORY ${SYSROOT}/boot)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${SYSROOT}/boot)

############################### Adding Sources ###############################

# NOTE: kernel should not include any architecture specific code
# including assembly
alkos_find_sources(KERNEL_SOURCES
    SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

if (CMAKE_FEATURE_FLAG_RUN_TEST_MODE)
    file(GLOB_RECURSE KERNEL_TEST_SOURCES
            "${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/test/*.c"
    )

    # Append test sources to the kernel sources
    list(APPEND KERNEL_SOURCES ${KERNEL_TEST_SOURCES})
endif ()

#################################### Exec ####################################

add_executable(alkos.kernel
        ${KERNEL_SOURCES}
)

############################### Adding Headers ###############################

target_include_directories(alkos.kernel.headers INTERFACE
    include
    test
    abi
)

########################## Configure for given Arch ##########################

add_subdirectory(arch/${ARCH})

# TODO: Immediately here, error checking should happen
# But the error checking itself fails!!! and for some reason 
# alkos_ensure_property_defined() says the property is not defined

################################# ThirdParty #################################

add_subdirectory(thirdparty)

target_link_libraries(alkos.kernel.deps INTERFACE
    alkos.kernel.thirdparty
    libk
    AutoGenLib
)

############## Linker Configuration for CXX Global Constructors ##############
# NOTE: This linking must be done at this level as global constructors
# should be supported on each architecture

# Retrieve our custom CRT object paths from the arch-specific configuration.
get_property(CRTI_OBJ_PATH TARGET alkos.kernel.config PROPERTY CRTI_OBJ)
get_property(CRTN_OBJ_PATH TARGET alkos.kernel.config PROPERTY CRTN_OBJ)
alkos_ensure_defined(
    VARS CRTI_OBJ_PATH CRTN_OBJ_PATH
    MESSAGE "CRTI_OBJ/CRTN_OBJ properties were not set by the architecture-specific CMake file."
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

# !!! Debug start !!!
message(STATUS "!!! DEBUG START !!!")
message(STATUS "DEBUG: CMAKE_C_COMPILER is ${CMAKE_C_COMPILER}")
message(STATUS "DEBUG: Output of crtbegin search: '${CRTBEGIN_OBJ_PATH}'")
message(STATUS "DEBUG: Error of crtbegin search: '${CRTBEGIN_ERROR}'")
message(STATUS "DEBUG: Output of crtend search: '${CRTEND_OBJ_PATH}'")
message(STATUS "DEBUG: Error of crtend search: '${CRTEND_ERROR}'")
message(STATUS "!!! DEBUG END !!!")
# !!! Debug end !!!

alkos_ensure_defined(
    VARS CRTBEGIN_OBJ_PATH CRTEND_OBJ_PATH
    MESSAGE "Could not find compiler's crtbegin.o/crtend.o files."
)

message(STATUS "Found CRT Objects:")
message(STATUS "  crti: ${CRTI_OBJ_PATH}")
message(STATUS "  crtbegin: ${CRTBEGIN_OBJ_PATH}")
message(STATUS "  crtend: ${CRTEND_OBJ_PATH}")
message(STATUS "  crtn: ${CRTN_OBJ_PATH}")

# Add all objects and libraries to the kernel target in the correct link order.
#    The linker requires: crti, crtbegin, <main objects>, <libraries>, crtend, crtn
#    target_link_libraries preserves this order.
target_link_libraries(alkos.kernel PRIVATE
    target.properties

    ${CRTI_OBJ_PATH}
    ${CRTBEGIN_OBJ_PATH}

    # Order is important here!
    # Do not link ANYTHING before crtbegin.o
    alkos.kernel.headers
    alkos.kernel.deps

    ${CRTEND_OBJ_PATH}
    ${CRTN_OBJ_PATH}

    gcc
)
