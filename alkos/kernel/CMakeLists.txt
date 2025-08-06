message(STATUS "Configuring kernel...")

# This target acts as a central hub for configuration data (e.g., kernel modules)
# that needs to be communicated to the top-level CMakeLists.txt.
add_library(alkos.kernel.config INTERFACE)
add_library(alkos.kernel.headers INTERFACE)  # Kernel headers for exporting
add_library(alkos.kernel.deps INTERFACE)     # Target to link all dependencies to
#------------------------------------------------------------------------------#
#                           Notes: alkos.kernel.deps                           #
#------------------------------------------------------------------------------#
# Note: Caveat - linking objects to this target also requires
# using target_sources $<TARGET_OBJECTS:{some lib name}> syntax because
# interface libraries are only carriers for headers and properties
#
#------------------------------------------------------------------------------#
# Note: This target is used to ensure proper linking order 
# Idea was that this is the linking "api" of the kernel
# So that arch etc, doesn't have to care about the linking order
# This is current unused in the intended way, but may be useful in the future
#
#------------------------------------------------------------------------------#

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

add_executable(alkos.kernel ${KERNEL_SOURCES})

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

)

################################# ThirdParty #################################

add_subdirectory(thirdparty)

target_link_libraries(alkos.kernel.deps INTERFACE
    alkos.kernel.thirdparty
    libk
    AutoGenLib
)

################################ Linking Kernel ################################

target_link_libraries(alkos.kernel PRIVATE
    target.properties
    alkos.kernel.config
    alkos.kernel.headers
    alkos.kernel.deps

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

alkos_ensure_defined(
    VARS CRTBEGIN_OBJ_PATH CRTEND_OBJ_PATH
    MESSAGE "Could not find compiler's crtbegin.o/crtend.o files."
)

message(STATUS "Found CRT Objects:")
message(STATUS "  crti: ${CRTI_OBJ_PATH}")
message(STATUS "  crtbegin: ${CRTBEGIN_OBJ_PATH}")
message(STATUS "  crtend: ${CRTEND_OBJ_PATH}")
message(STATUS "  crtn: ${CRTN_OBJ_PATH}")

# TODO:
# The below code could technically be replaced with target_link_libraries()
# it also should enforce the correct linking order.
# I have done so with success, but it introduced a bug where no tests were 
# detected in the kernel and I was unable to reslove it so gave up for now.

# Add all objects and libraries to the kernel target in the correct link order.
#    The linker requires: crti, crtbegin, <main objects>, <libraries>, crtend, crtn
set(CMAKE_CXX_LINK_EXECUTABLE
  "${CMAKE_CXX_COMPILER} <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> ${CRTI_OBJ_PATH} ${CRTBEGIN_OBJ_PATH} <OBJECTS> ${CRTEND_OBJ_PATH} ${CRTN_OBJ_PATH} -o <TARGET> <LINK_LIBRARIES>"
)

