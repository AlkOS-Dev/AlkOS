#
# This module provides functions to validate that critical CMake variables
# are defined, halting or warning the user if they are not.
#

#===============================================================================
# alkos_ensure_defined
#===============================================================================
#
# Checks if one or more variables are defined and halts with a fatal error,
# listing all undefined variables at once.
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
        if(NOT DEFINED ${var_name})
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
