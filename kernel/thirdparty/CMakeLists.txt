message(STATUS "  -> Adding third-party libraries...")
include(FetchContent)

add_library(alkos.kernel.thirdparty INTERFACE)

# Iterate over all dependencies and add them to the thirdparty library
file(GLOB directories RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)
foreach(dir ${directories})
    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${dir})
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/${dir})
    endif()
endforeach()
