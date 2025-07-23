############################### Configuring files ###############################

file(GLOB_RECURSE LIB_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

############################### Preparing target ###############################

add_library(AutoGenLib INTERFACE include)
