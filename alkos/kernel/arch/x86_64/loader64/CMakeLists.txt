message(STATUS "Configuring x86_64 64-bit loader")

#################################### Exec ####################################

add_executable(alkos.loader64 "")

############################### Adding Sources ###############################

alkos_target_sources(alkos.loader64)

############################### Adding Headers ###############################

target_include_directories(alkos.loader64 PRIVATE
        .
)

################################ Linker Flags ################################

set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/linker.ld")
target_link_options(alkos.loader64 PRIVATE
        -T ${LINKER_SCRIPT}     # Linker script
)

############################### Link Libraries ###############################

target_link_libraries(alkos.loader64 PRIVATE
        target.properties
        libk
        arch.common.all.64
        arch.common.kernel-loader.64
        alkos.kernel.headers
        AutoGenLib
        gcc
)
