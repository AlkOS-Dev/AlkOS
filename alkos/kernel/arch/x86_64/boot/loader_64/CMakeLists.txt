message(STATUS "      -> Configuring 64-bit bootloader")

set(CMAKE_VERBOSE_MAKEFILE ON)

#################################### Exec ####################################

add_executable(alkos.boot.loader.64)

alkos_target_sources(alkos.boot.loader.64)

target_include_directories(alkos.boot.loader.64 PRIVATE
        .
)
set_target_properties(alkos.boot.loader.64 PROPERTIES POSITION_INDEPENDENT_CODE ON) 

################################ Linker Flags ################################

set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/build/linker.ld")

target_link_options(alkos.boot.loader.64 PRIVATE
        -T ${LINKER_SCRIPT}     
        -fPIE
        -pie
        "LINKER:-z,norelro"
)

############################### Link Libraries ###############################

target_link_libraries(alkos.boot.loader.64 PRIVATE
        alkos.target.properties.pic.interface
        alkos.autogen.interface
        alkos.boot.lib.interface
        c.core.pic
        gcc
)
