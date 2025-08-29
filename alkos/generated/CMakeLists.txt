############################### Preparing target ###############################

add_library(alkos.autogen.interface INTERFACE)

target_include_directories(alkos.autogen.interface PRIVATE INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include/")
