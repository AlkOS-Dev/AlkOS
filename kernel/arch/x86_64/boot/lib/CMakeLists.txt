message(STATUS "      -> Configuring bootloader library")

add_library(alkos.boot.lib.interface INTERFACE)

alkos_find_sources(BOOT_LIB_SOURCES)

target_sources(alkos.boot.lib.interface INTERFACE ${BOOT_LIB_SOURCES})
target_include_directories(alkos.boot.lib.interface INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/../settings
        .
)

target_link_libraries(alkos.boot.lib.interface INTERFACE
    alkos.libcontainers.interface
)
