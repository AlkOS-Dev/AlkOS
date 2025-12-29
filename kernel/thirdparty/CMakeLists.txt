message(STATUS "  -> Adding third-party libraries...")
include(FetchContent)
include(MiscHelpers)

add_library(alkos.kernel.thirdparty INTERFACE)

# Iterate over all dependencies and add them to the thirdparty library
alkos_add_subdirs(${CMAKE_CURRENT_SOURCE_DIR})
