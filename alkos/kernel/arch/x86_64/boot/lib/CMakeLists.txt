add_library(boot.lib.interface INTERFACE)

alkos_find_sources(BOOT_LIB_SOURCES
    EXCLUDE ".*/tests/.*"
)
target_sources(boot.lib.interface INTERFACE ${BOOT_LIB_SOURCES})
target_include_directories(boot.lib.interface INTERFACE
        .
)
