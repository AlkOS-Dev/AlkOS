#
# This module provides functions to validate that critical CMake variables
# are defined, halting or warning the user if they are not.
#

#===============================================================================
# alkos_ensure_defined
#===============================================================================
#
# Checks if one or more variables are defined (and nonempty) and halts with a 
# fatal error, listing all undefined variables at once.
#
# Parameters:
#   VARS <var>...          A list of variable names to check.
#   MESSAGE <message>      (Optional) A supplementary message to append to the
#                          standard error output. This is useful for providing
#                          additional context or instructions to the user.
#
# Example:
#   # Assume ARCH is defined but SYSROOT is not.
#   alkos_ensure_defined(
#       VARS SYSROOT ARCH
#       MESSAGE "The SYSROOT variable must be set to the toolchain's root path."
#   )
#
#   # The above call would produce the following fatal error:
#   #
#   #   CMake Error at CMakeLists.txt:XX (message):
#   #     The following required variables are not defined: SYSROOT
#   #     The SYSROOT variable must be set to the toolchain's root path.
#
function(alkos_ensure_defined)
    set(options)
    set(oneValueArgs MESSAGE)
    set(multiValueArgs VARS)

    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT ARG_VARS)
        message(FATAL_ERROR "alkos_ensure_defined() called without any VARS to check.")
    endif()

    set(undefined_vars "")
    foreach(var_name ${ARG_VARS})
        if(NOT ${var_name})
            list(APPEND undefined_vars "${var_name}")
        endif()
    endforeach()

    if(undefined_vars)
        string(JOIN ", " undefined_vars_str "${undefined_vars}")

        set(error_message "The following required variables are not defined: ${undefined_vars_str}")

        if(ARG_MESSAGE)
            string(APPEND error_message "\n${ARG_MESSAGE}")
        endif()

        message(FATAL_ERROR "${error_message}")
    endif()
endfunction()

#===============================================================================
# alkos_ensure_property_defined
#===============================================================================
#
# Checks if a property is defined for a target and halts with a fatal error,
# listing all undefined properties at once.
#
# Parameters:
#   TARGET <target>        The target to check the properties for.
#   PROPS <prop>...        A list of property names to check.
#   MESSAGE <message>      (Optional) A supplementary message to append to the
#                          standard error output. This is useful for providing
#                          additional context or instructions to the user.
#
# Example:
#   # Assume alkos.kernel.config is a target and BOOTABLE_KERNEL_EXECUTABLE
#   # is not defined.
#   alkos_ensure_property_defined(
#       TARGET alkos.kernel.config
#       PROPS BOOTABLE_KERNEL_EXECUTABLE KERNEL_MODULES
#       MESSAGE "The BOOTABLE_KERNEL_EXECUTABLE property must be set for the KERNEL_MODULES target."
#   )
#
function(alkos_ensure_property_defined)
    set(options)
    set(oneValueArgs TARGET MESSAGE)
    set(multiValueArgs PROPS)

    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT ARG_TARGET)
        message(FATAL_ERROR "alkos_ensure_property_defined() called without a TARGET to check.")
    endif()

    if(NOT ARG_PROPS)
        message(FATAL_ERROR "alkos_ensure_property_defined() called without any PROPS to check.")
    endif()

    set(undefined_props "")
    foreach(prop_name ${ARG_PROPS})
        get_property(is_defined TARGET ${ARG_TARGET} PROPERTY ${prop_name} DEFINED)
        if(NOT is_defined)
            list(APPEND undefined_props "${prop_name}")
        endif()
    endforeach()

    if(undefined_props)
        string(JOIN ", " undefined_props_str "${undefined_props}")

        set(error_message "The following required properties are not defined for target '${ARG_TARGET}': ${undefined_props_str}")

        if(ARG_MESSAGE)
            string(APPEND error_message "\n${ARG_MESSAGE}")
        endif()

        message(FATAL_ERROR "${error_message}")
    endif()
endfunction()

#===============================================================================
# alkos_ensure_path_exists
#===============================================================================
#
# Checks if a specified path/paths exists and halts with a fatal error if it does not.
#
# Parameters:
#   PATHS <path>...        A list of paths to check for existence.
#   MESSAGE <message>      (Optional) A supplementary message to append to the
#                          standard error output. This is useful for providing
#                          additional context or instructions to the user.
#
# Example:
#   # Assume /usr/local/bin is a critical path that must exist.
#   alkos_ensure_path_exists(
#       PATHS /usr/local/bin
#       MESSAGE "The /usr/local/bin directory is required for the build process."
#   )
#
function(alkos_ensure_path_exists)
    set(options)
    set(oneValueArgs MESSAGE)
    set(multiValueArgs PATHS)

    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT ARG_PATHS)
        message(FATAL_ERROR "alkos_ensure_path_exists() called without any PATHS to check.")
    endif()

    set(nonexistent_paths "")
    foreach(path ${ARG_PATHS})
        if(NOT EXISTS ${path})
            list(APPEND nonexistent_paths "${path}")
        endif()
    endforeach()

    if(nonexistent_paths)
        string(JOIN ", " nonexistent_paths_str "${nonexistent_paths}")

        set(error_message "The following required paths do not exist: ${nonexistent_paths_str}")

        if(ARG_MESSAGE)
            string(APPEND error_message "\n${ARG_MESSAGE}")
        endif()

        message(FATAL_ERROR "${error_message}")
    endif()
endfunction()
