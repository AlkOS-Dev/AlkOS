#
# This module provides a function to register a complete, runnable
# environment for a given architecture.
#

include(ValidationHelpers)
include(BashConfigHelpers)

#===============================================================================
# alkos_register_runtime_environment
#===============================================================================
#
# This function registers a complete runtime environment for a specific architecture.
# It creates targets for building an ISO, running the OS in QEMU, and optionally
# running tests or debugging with GDB.
#
## Parameters:
#   ARCH_NAME             The name of the architecture (e.g., x86_64).
#   BOOTABLE_EXECUTABLE   The bootloader or initial kernel executable target.
#   QEMU_COMMAND          The QEMU command for the architecture (e.g., qemu-system-x86_64).
#   QEMU_NORMAL_FLAGS     Default flags for running the OS in QEMU.
#   QEMU_TEST_FLAGS       Specific flags for running tests in QEMU.
## Keyword Arguments:
#   MODULES               A list of kernel module targets to include in the ISO.
#   MODULE_COMMANDS       A list of corresponding commands for the bootloader menu.
## Example:
#   alkos_register_runtime_environment(
#       ARCH_NAME           x86_64
#       BOOTABLE_EXECUTABLE alkos.loader32
#       QEMU_COMMAND        "qemu-system-x86_64"
#       QEMU_NORMAL_FLAGS   "-m 4G -smp 4"
#       QEMU_TEST_FLAGS     "-m 4G -display none"
#       MODULES
#           alkos.loader64
#           alkos.kernel
#       MODULE_COMMANDS
#           "loader64"
#           "kernel"
#   )
#
function(alkos_register_runtime_environment)
    set(options)
    set(oneValueArgs
        ARCH_NAME
        BOOTABLE_EXECUTABLE
        QEMU_COMMAND
        QEMU_NORMAL_FLAGS
        QEMU_TEST_FLAGS
    )
    set(multiValueArgs
        MODULES
        MODULE_COMMANDS
    )

    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(MAKE_ISO_SCRIPT_PATH ${CMAKE_ROOT_DIR}/scripts/actions/make_iso.bash)
    set(RUN_ALKOS_SCRIPT_PATH ${CMAKE_ROOT_DIR}/scripts/actions/run_alkos.bash)
    set(RUN_TESTS_SCRIPT_PATH ${CMAKE_ROOT_DIR}/scripts/actions/run_tests.bash)
    set(ALKOS_ISO_PATH ${CMAKE_BINARY_DIR}/alkos-${ARG_ARCH_NAME}.iso)
    alkos_ensure_path_exists(
        PATHS ${MAKE_ISO_SCRIPT_PATH} ${RUN_ALKOS_SCRIPT_PATH} ${RUN_TESTS_SCRIPT_PATH}
    )

    # Add architecture-specific variables to the bash config
    alkos_add_to_bash_config("CONF_ARCH" "${ARG_ARCH_NAME}")
    alkos_add_to_bash_config("CONF_BUILD_TYPE" "${CMAKE_BUILD_TYPE}")
    alkos_add_to_bash_config("CONF_SYSROOT" "${CMAKE_SYSROOT}")
    alkos_add_to_bash_config("CONF_BOOTABLE_KERNEL_EXEC" "${ARG_BOOTABLE_EXECUTABLE}")
    alkos_add_to_bash_config("CONF_ISO_PATH" "${ALKOS_ISO_PATH}")
    alkos_bash_config_append_to_list("CONF_KERNEL_MODULES" "${ARG_MODULES}")
    alkos_bash_config_append_to_list("CONF_KERNEL_COMMANDS" "${ARG_MODULE_COMMANDS}")
    alkos_add_to_bash_config("CONF_BUILD_DIR" "${CMAKE_BINARY_DIR}")
    alkos_add_to_bash_config("CONF_TOOL_DIR" "${TOOL_BINARIES_DIR}")
    alkos_add_to_bash_config("CONF_QEMU_COMMAND" "${ARG_QEMU_COMMAND}")
    alkos_add_to_bash_config("CONF_QEMU_NORMAL_FLAGS" "${ARG_QEMU_NORMAL_FLAGS}")
    alkos_add_to_bash_config("CONF_QEMU_TEST_FLAGS" "${ARG_QEMU_TEST_FLAGS}")

    add_custom_target(iso-${ARG_ARCH_NAME}
        COMMAND ${MAKE_ISO_SCRIPT_PATH} -v
        DEPENDS ${ARG_BOOTABLE_EXECUTABLE} ${ARG_MODULES} 
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Building ISO for ${ARG_ARCH_NAME}"
    )

    add_custom_target(run-${ARG_ARCH_NAME}
        COMMAND ${RUN_ALKOS_SCRIPT_PATH} -v
        DEPENDS iso-${ARG_ARCH_NAME}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Running AlkOS for ${ARG_ARCH_NAME}"
    )

    add_custom_target(run-with-gdb-${ARG_ARCH_NAME}
        COMMAND ${RUN_ALKOS_SCRIPT_PATH} -v --gdb 
        DEPENDS iso-${ARG_ARCH_NAME}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Running AlkOS with GDB for ${ARG_ARCH_NAME}"
    )

    add_custom_target(mount-${ARG_ARCH_NAME}
        COMMAND ${RUN_ALKOS_SCRIPT_PATH} -v --mount 
        DEPENDS iso-${ARG_ARCH_NAME}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Mounting AlkOS for ${ARG_ARCH_NAME}"
    )

    if (CMAKE_FEATURE_FLAG_RUN_TEST_MODE)
        separate_arguments(SPLIT_TEST_ARGS UNIX_COMMAND "${CMAKE_TEST_ARGS}")
        add_custom_target(tests-${ARG_ARCH_NAME}
          COMMAND ${RUN_TESTS_SCRIPT_PATH} ${SPLIT_TEST_ARGS}
          DEPENDS iso-${ARG_ARCH_NAME}
          WORKING_DIRECTORY ${CMAKE_ROOT_DIR}/scripts/tests
          COMMENT "Running tests for AlkOS on ${ARG_ARCH_NAME}"
      )
    endif()
endfunction()
#-------------------------------------------------------------------------------
# register_filesystem
#-------------------------------------------------------------------------------
# A filesystem registration helper that configures the rootfs
# generation. It can be used to create either an initrd within the sysroot
# or a standalone filesystem image within the build binaries folder.
#
# Parameters (oneValueArgs):
#   TYPE                  Either INITRD or IMAGE
#   ROOTFS_TYPE           Type of rootfs to create (e.g., fat, ext4)
#   ROOTFS_DIR            Staging directory for building rootfs contents
#   ROOTFS_TARGET_PATH    Target image path to generate (optional; defaults based on TYPE)
#   ROOTFS_OVERLAY_DIR    Overlay directory to copy into the rootfs
#   MODULE                Optional module target name to include in the ISO
#   MODULE_COMMAND        Optional bootloader command for the module
#
#-------------------------------------------------------------------------------
function(register_filesystem)
    set(options)
    set(oneValueArgs
        TYPE
        ROOTFS_TYPE
        ROOTFS_TARGET_PATH
        ROOTFS_OVERLAY_DIR
        MODULE
        MODULE_COMMAND
    )
    set(multiValueArgs)

    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    alkos_ensure_called_once(NAME ${CMAKE_CURRENT_FUNCTION})

    alkos_ensure_defined(
        VARS ARG_TYPE ARG_ROOTFS_TYPE ARG_ROOTFS_TARGET_PATH ARG_ROOTFS_OVERLAY_DIR
    )

    # Resolve target defaults and feature flags
    if(ARG_TYPE STREQUAL "INITRD")
        alkos_ensure_feature_enabled(
            FLAG CMAKE_FEATURE_FLAG_RAMDISK
            MESSAGE "Initial RAM filesystem support is disabled."
        )
    elseif(ARG_TYPE NOT STREQUAL "IMAGE")
        message(FATAL_ERROR "register_filesystem: Unknown TYPE='${ARG_TYPE}'. Expected INITRD or IMAGE.")
    endif()

    # Export variables for the rootfs maker
    alkos_add_to_bash_config("CONF_ROOTFS_TARGET_PATH" "${ARG_ROOTFS_TARGET_PATH}")
    alkos_add_to_bash_config("CONF_ROOTFS_OVERLAY_DIR" "${ARG_ROOTFS_OVERLAY_DIR}")

    if(ARG_TYPE STREQUAL "INITRD")
        alkos_ensure_defined(VARS ARG_MODULE ARG_MODULE_COMMAND)

        alkos_bash_config_append_to_list("CONF_KERNEL_MODULES" "${ARG_MODULE}")
        alkos_bash_config_append_to_list("CONF_KERNEL_COMMANDS" "${ARG_MODULE_COMMAND}")
    endif()

    # Create the targets
    set(MAKE_ROOTFS_SCRIPT_PATH ${CMAKE_SOURCE_DIR}/scripts/actions/make_rootfs.bash)
    alkos_ensure_path_exists(PATHS ${MAKE_ROOTFS_SCRIPT_PATH})

    add_custom_target(rootfs
            # Ensure the staging directory is clean before building the rootfs.
            # Remove the directory and recreate it so the generator starts from
            # an empty staging area.
            COMMAND ${CMAKE_COMMAND} -E rm -rf "${ROOTFS_DIR}"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${ROOTFS_DIR}"
            COMMAND ${MAKE_ROOTFS_SCRIPT_PATH} -v -- ${ARG_ROOTFS_TYPE}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Building root filesystem"
    )
    add_dependencies(iso-${ARCH} rootfs)

    add_custom_target(mountfs
            DEPENDS rootfs
            COMMAND sudo umount /mnt || true
            COMMAND sudo mount -o loop ${ARG_ROOTFS_TARGET_PATH} /mnt
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Mounting root filesystem"
    )
endfunction()
