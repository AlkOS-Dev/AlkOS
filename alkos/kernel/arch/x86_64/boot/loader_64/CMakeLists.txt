message(STATUS "Configuring x86_64 64-bit loader")

#################################### Exec ####################################

add_executable(alkos.boot.loader.64)

alkos_target_sources(alkos.boot.loader.64)

target_include_directories(alkos.boot.loader.64 PRIVATE
        .
)

################################ Linker Flags ################################

set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/build/linker.ld")
target_link_options(alkos.boot.loader.64 PRIVATE
        -T ${LINKER_SCRIPT}     
)

############################### Link Libraries ###############################

target_link_libraries(alkos.boot.loader.64 PRIVATE
        alkos.target.properties.interface
        alkos.autogen.interface
        alkos.boot.lib.interface
        c.core
        gcc
)
