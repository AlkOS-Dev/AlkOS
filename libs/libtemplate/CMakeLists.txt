add_library(alkos.libtemplate.interface INTERFACE)

target_include_directories(alkos.libtemplate.interface
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(alkos.libtemplate.interface
    INTERFACE
        alkos.libc.interface
)
