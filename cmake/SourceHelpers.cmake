#
# This module provides functions to simplify finding and adding source files
# to targets or variables.
#

include(ValidationHelpers)

#===============================================================================
# alkos_find_sources
#===============================================================================
#
# Finds source files based on patterns and stores them in a variable.
#
# Parameters:
#   <output_var>      The variable to store the list of found files in.
#
# Keyword Arguments:
#   SOURCES <pattern>...  A list of globbing patterns to search for.
#                         If omitted, it defaults to searching for the following
#                         extensions in the current directory:
#                         .c, .cpp, .s, .S, .asm, .nasm
#   EXCLUDE <regex>...    A list of regular expressions to exclude from the results.
#
# Example (Default):
#   # Finds all default source types in the current directory
#   alkos_find_sources(MY_SOURCES)
#
# Example (Custom):
#   alkos_find_sources(MY_SOURCES
#       SOURCES
#           "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
#       EXCLUDE
#           ".*/temp/.*"
#   )
#
function(alkos_find_sources output_var)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs SOURCES EXCLUDE)

    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "alkos_find_sources() called with unparsed arguments: ${ARG_UNPARSED_ARGUMENTS}")
    endif()

    set(source_patterns "")

    if(NOT ARG_SOURCES)
        # Default
        set(default_extensions "cpp;c;s;S;asm;nasm")
        foreach(ext ${default_extensions})
            list(APPEND source_patterns "${CMAKE_CURRENT_SOURCE_DIR}/*.${ext}")
        endforeach()
    else()
        # Use the user-provided source patterns.
        set(source_patterns ${ARG_SOURCES})
    endif()


    file(GLOB_RECURSE found_files ${source_patterns})

    if(ARG_EXCLUDE)
        foreach(exclude_regex ${ARG_EXCLUDE})
            list(FILTER found_files EXCLUDE REGEX "${exclude_regex}")
        endforeach()
    endif()

    set(${output_var} ${found_files} PARENT_SCOPE)
endfunction()

#===============================================================================
# alkos_target_sources
#===============================================================================
#
# Finds source files based on patterns and adds them to a target.
#
# This function is a wrapper around alkos_find_sources and target_sources
# to simplify adding sources directly to a target.
#
# Parameters:
#   <target>          The target to add the source files to.
#
# Keyword Arguments:
#   SOURCES <pattern>...  A list of globbing patterns to search for.
#   EXCLUDE <regex>...    A list of regular expressions to exclude from the results.
#
# Example:
#   alkos_target_sources(MyExecutable
#       SOURCES
#           "src/*.cpp"
#           "src/*.c"
#       EXCLUDE
#           "src/experimental/.*"
#   )
#
function(alkos_target_sources target)
    if(NOT TARGET ${target})
        message(FATAL_ERROR "alkos_target_sources: The target '${target}' does not exist.")
    endif()

    alkos_find_sources(source_list ${ARGN})

    if(source_list)
        target_sources(${target} PRIVATE ${source_list})
    endif()
endfunction()
