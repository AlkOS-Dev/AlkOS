#===============================================================================
# Define MOCK Interfaces (Dependency Injection)
#===============================================================================
# We create the targets that libcontainers and libtemplate and others 
# EXPECT to find.
# We point them to our 'mocks' folder.
#-------------------------------------------------------------------------------

add_library(alkos.libc.interface INTERFACE)
target_include_directories(alkos.libc.interface INTERFACE
    .
)

add_library(alkos.libcpp.interface INTERFACE)
target_include_directories(alkos.libcpp.interface INTERFACE
    .
)

target_link_libraries(alkos.libcpp.interface INTERFACE alkos.libc.interface)
