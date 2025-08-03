message(STATUS "Configuring kernel...")

############################### Error Checking ###############################

if (NOT DEFINED SYSROOT)
    message(FATAL_ERROR "SYSROOT is not defined")
endif ()

if (NOT DEFINED ARCH)
    message(FATAL_ERROR "ARCH is not defined")
endif ()

############################ Compile Definitions #############################

add_compile_definitions(__ALKOS_KERNEL__=1)
add_compile_definitions(__ALKOS_LIBK__=1) # For LIBK headers

############################# Arch Dependent Vars ############################

# ----------------------------------------------------------------------------
# Arch can optionally create other executables (for chainloading, etc)
# They need to place themselves in sysroot/boot
# This is the list of executables that will be loaded by the bootloader as
# modules
set(KERNEL_MODULES "")
# ----------------------------------------------------------------------------
# This is the executable that the bootloader will give control to
set(BOOTABLE_KERNEL_EXECUTABLE alkos.kernel)
# ----------------------------------------------------------------------------

########################### Setting Sysroot Boot #############################

file(MAKE_DIRECTORY ${SYSROOT}/boot)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${SYSROOT}/boot)

############################### Adding Sources ###############################

# NOTE: kernel should not include any architecture specific code
# including assembly
file(GLOB_RECURSE KERNEL_SOURCES
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

add_library(alkos.kernel.headers INTERFACE)
target_include_directories(alkos.kernel.headers INTERFACE
    include
    test
    abi
)

target_link_libraries(alkos.kernel PRIVATE
    alkos.kernel.headers
)

########################## Configure for given Arch ##########################

add_subdirectory(arch/${ARCH})

############################### Error Checking ###############################

if (NOT DEFINED BOOTABLE_KERNEL_EXECUTABLE)
    message(FATAL_ERROR "No primary kernel executable defined")
endif ()

message(STATUS "Bootable kernel executable: ${BOOTABLE_KERNEL_EXECUTABLE}")
message(STATUS "Kernel modules: ${KERNEL_MODULES}")
message(STATUS "Kernel commands: ${KERNEL_COMMANDS}")

# Check if KERNEL_MODULES and KERNEL_COMMANDS are the same size and defined
if (NOT DEFINED KERNEL_COMMANDS OR NOT DEFINED KERNEL_MODULES)
    message(FATAL_ERROR "KERNEL_MODULES or KERNEL_COMMANDS are not defined")
endif ()

list(LENGTH KERNEL_MODULES KERNEL_MODULES_LENGTH)
list(LENGTH KERNEL_COMMANDS KERNEL_COMMANDS_LENGTH)

# Check if the two lists are the same size
if(NOT KERNEL_MODULES_LENGTH EQUAL KERNEL_COMMANDS_LENGTH)
    message(FATAL_ERROR "KERNEL_MODULES and KERNEL_COMMANDS have different sizes: ${KERNEL_MODULES_LENGTH} vs ${KERNEL_COMMANDS_LENGTH}")
endif()

if(NOT DEFINED ARCH_QEMU_COMMAND OR NOT DEFINED ARCH_QEMU_NORMAL_FLAGS OR NOT DEFINED ARCH_QEMU_TEST_FLAGS)
    message(FATAL_ERROR "ARCH_QEMU_COMMAND, ARCH_QEMU_NORMAL_FLAGS, ARCH_QEMU_TEST_FLAGS are not defined")
endif ()

################################# ThirdParty #################################

message(STATUS "Adding third-party libraries...")
add_subdirectory(thirdparty)

################################## Linking ###################################

target_link_libraries(alkos.kernel PRIVATE libk gcc ${THIRD_PARTY_LIBRARIES} AutoGenLib)

############################## Post Arch Action ##############################

# Note: architecture file has a possibility to define custom command to run
# before linking
if (DEFINED POST_ARCH_ACTION)
    message(STATUS "POST ACTION: ${POST_ARCH_ACTION}")

    add_custom_command(TARGET alkos.kernel PRE_LINK
            COMMAND ${POST_ARCH_ACTION}
            COMMENT "Running post build arch action"
    )
endif ()

############## Linker Configuration for CXX Global Constructors ##############
# Set the linker to link objects in the correct order
# NOTE: ARCH cmake is expected to provide variables for global constructors
# according to the compiler needs
# NOTE: This linking must be done at this level as global constructors
# should be supported on each architecture
if (NOT DEFINED CRTI_OBJ  OR NOT DEFINED CRTN_OBJ)
    message(FATAL_ERROR "One of: crti.o, crtn.o is not defined. Those should be defined in the ARCH CMakeLists.txt file")
endif ()

set(CMAKE_CXX_LINKER_LAUNCHER "/bin/bash -c")
set(CMAKE_CXX_LINK_EXECUTABLE
"/bin/bash -c \"${CMAKE_CXX_COMPILER} <CMAKE_CXX_LINK_FLAGS> <FLAGS> <LINK_FLAGS> \
${CRTI_OBJ} \
\$\(${CMAKE_C_COMPILER} -print-file-name=crtbegin.o\) \
<OBJECTS> \
\$\(${CMAKE_C_COMPILER} -print-file-name=crtend.o\) \
${CRTN_OBJ} \
-o <TARGET> <LINK_LIBRARIES>\""
)

########################## Back-Propagate Variables ##########################

set(BOOTABLE_KERNEL_EXECUTABLE ${BOOTABLE_KERNEL_EXECUTABLE} PARENT_SCOPE)
set(KERNEL_MODULES ${KERNEL_MODULES} PARENT_SCOPE)
set(KERNEL_COMMANDS ${KERNEL_COMMANDS} PARENT_SCOPE)

set(ARCH_QEMU_COMMAND ${ARCH_QEMU_COMMAND} PARENT_SCOPE)
set(ARCH_QEMU_NORMAL_FLAGS ${ARCH_QEMU_NORMAL_FLAGS} PARENT_SCOPE)
set(ARCH_QEMU_TEST_FLAGS ${ARCH_QEMU_TEST_FLAGS} PARENT_SCOPE)
