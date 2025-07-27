############################### Preparing target ###############################

add_library(AutoGenLib INTERFACE)

target_include_directories(AutoGenLib PRIVATE INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include/")
