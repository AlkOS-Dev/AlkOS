message(STATUS "Configuring kernel...")

# TODO: This currently works as interface to attach crti and crtn object files
# And should probably be kept that way and renamed
add_library(alkos.kernel.config INTERFACE)

############################### Error Checking ###############################

alkos_ensure_defined(
    VARS SYSROOT ARCH
)

############################ Compile Definitions #############################

add_compile_definitions(__ALKOS_KERNEL__=1)

########################### Setting Sysroot Boot #############################

file(MAKE_DIRECTORY ${SYSROOT}/boot)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${SYSROOT}/boot)

#################################### Exec ####################################

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

    list(APPEND KERNEL_SOURCES ${KERNEL_TEST_SOURCES})
endif ()

add_executable(alkos.kernel ${KERNEL_SOURCES})

############################### Adding Headers ###############################

target_include_directories(alkos.kernel PUBLIC
    src
    test
)

############################# Libc Instantiation #############################

add_library(c STATIC ${ALKOS_LIBC_SOURCES})

target_link_libraries(c PUBLIC
    alkos.libc.interface
    alkos.target.properties.interface
    gcc
)

target_compile_definitions(c PRIVATE __ALKOS_KERNEL_LIBC__)

########################## Configure for given Arch ##########################

add_subdirectory(arch/${ARCH})

# TODO: Immediately here, error checking should happen
# But the error checking itself fails!!! and for some reason
# alkos_ensure_property_defined() says the property is not defined

################################# ThirdParty #################################

add_subdirectory(thirdparty)

################################ Linking Kernel ################################

target_link_libraries(alkos.kernel PRIVATE
    alkos.target.properties.interface
    alkos.kernel.config
    alkos.kernel.thirdparty
    alkos.autogen.interface
    c
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
    RESULT_VARIABLE CRTBEGIN_RESULT
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
    COMMAND ${CMAKE_C_COMPILER} -print-file-name=crtend.o
    OUTPUT_VARIABLE CRTEND_OBJ_PATH
    ERROR_VARIABLE CRTEND_ERROR
    RESULT_VARIABLE CRTEND_RESULT
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(CRTBEGIN_RESULT OR CRTEND_RESULT)
    message(FATAL_ERROR "Failed to find compiler's CRT files.\\n"
        "crtbegin command: ${CMAKE_C_COMPILER} -print-file-name=crtbegin.o\\n"
        "crtbegin result: ${CRTBEGIN_RESULT}\\n"
        "crtbegin stderr: ${CRTBEGIN_ERROR}\\n"
        "crtend command: ${CMAKE_C_COMPILER} -print-file-name=crtend.o\\n"
        "crtend result: ${CRTEND_RESULT}\\n"
        "crtend stderr: ${CRTEND_ERROR}\\n"
    )
endif()

alkos_ensure_defined(
    VARS CRTBEGIN_OBJ_PATH CRTEND_OBJ_PATH
    MESSAGE "Could not find compiler's crtbegin.o/crtend.o files (command succeeded but produced no output)."
)

message(STATUS "  [INFO] Found CRT Objects:")
message(STATUS "    crti:     ${CRTI_OBJ_PATH}")
message(STATUS "    crtbegin: ${CRTBEGIN_OBJ_PATH}")
message(STATUS "    crtend:   ${CRTEND_OBJ_PATH}")
message(STATUS "    crtn:     ${CRTN_OBJ_PATH}")

# Add all objects and libraries to the kernel target in the correct link order.
#    The linker requires: crti, crtbegin, <main objects>, <libraries>, crtend, crtn
set(CMAKE_CXX_LINK_EXECUTABLE
  "${CMAKE_CXX_COMPILER} <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> ${CRTI_OBJ_PATH} ${CRTBEGIN_OBJ_PATH} <OBJECTS> ${CRTEND_OBJ_PATH} ${CRTN_OBJ_PATH} -o <TARGET> <LINK_LIBRARIES>"
)
