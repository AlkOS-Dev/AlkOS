#
# This module provides functions to validate that critical CMake variables
# are defined, halting or warning the user if they are not.
#

#===============================================================================
# alkos_ensure_defined
#===============================================================================
#
# Checks if one or more variables are defined and halts with a fatal error
# if any are not.
#
# Parameters:
#   VARS <var>...          A list of variable names to check.
#   MESSAGE <message>      The error message to display if a variable is not
#                          defined. The message can include the placeholder
#                          <VAR_NAME> which will be replaced with the name of
#                          the undefined variable.
#
# Example:
#   alkos_ensure_defined(
#       VARS SYSROOT ARCH
#       MESSAGE "Configuration error: The variable <VAR_NAME> is not defined."
#   )
#
function(alkos_ensure_defined)
    set(options)
    set(oneValueArgs MESSAGE)
    set(multiValueArgs VARS)

    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT ARG_VARS)
        message(FATAL_ERROR "alkos_ensure_defined() called without any VARS to check.")
    endif()

    if(NOT ARG_MESSAGE)
        message(FATAL_ERROR "alkos_ensure_defined() called without a MESSAGE.")
    endif()

    foreach(var_name ${ARG_VARS})
        if(NOT DEFINED ${var_name})
            string(REPLACE "<VAR_NAME>" "${var_name}" error_message "${ARG_MESSAGE}")
            message(FATAL_ERROR "${error_message}")
        endif()
    endforeach()
endfunction()
