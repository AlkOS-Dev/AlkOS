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
#   alkos_add_subdirs(modules)
#   alkos_add_subdirs(${CMAKE_CURRENT_SOURCE_DIR}/modules)
#
function(alkos_add_subdirs BASE_DIR)
    if(IS_ABSOLUTE "${BASE_DIR}")
        set(base_dir "${BASE_DIR}")
    else()
        set(base_dir "${CMAKE_CURRENT_SOURCE_DIR}/${BASE_DIR}")
    endif()

    file(GLOB subdirs RELATIVE "${base_dir}" "${base_dir}/*")
    foreach(subdir ${subdirs})
        if(IS_DIRECTORY "${base_dir}/${subdir}")
            add_subdirectory("${base_dir}/${subdir}")
        endif()
    endforeach()
endfunction()
