#
# Miscellaneous CMake helper functions
#

#===============================================================================
# alkos_add_subdirs
#===============================================================================
# Recursively adds all subdirectories in the given BASE_DIR.
## Parameters:
#   BASE_DIR          The base directory to search for subdirectories.
## Example:
#   alkos_add_subdirs(${CMAKE_CURRENT_SOURCE_DIR}/modules)
#
function(alkos_add_subdirs BASE_DIR)
    file(GLOB directories RELATIVE "${BASE_DIR}" "${BASE_DIR}/*")
    foreach(dir ${directories})
        if(IS_DIRECTORY "${BASE_DIR}/${dir}")
            add_subdirectory("${BASE_DIR}/${dir}")
        endif()
    endforeach()
endfunction()
