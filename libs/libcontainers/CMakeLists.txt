add_library(alkos.libcontainers.interface INTERFACE)

target_include_directories(alkos.libcontainers.interface
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(alkos.libcontainers.interface
    INTERFACE
        alkos.libcpp.interface
        alkos.libtemplate.interface
)
