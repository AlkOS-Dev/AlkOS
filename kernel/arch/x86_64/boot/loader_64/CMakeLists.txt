message(STATUS "      -> Configuring 64-bit bootloader")

############################# Libc Instantiation #############################

# Create a PIC version of libc for the bootloader.
add_library(c.pic STATIC ${ALKOS_LIBC_SOURCES})
set_target_properties(c.pic PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(c.pic PUBLIC
    alkos.libc.interface
    alkos.target.properties.pic.interface
    gcc
)
target_compile_definitions(c.pic PRIVATE __ALKOS_KERNEL_LIBC__)

# Create a PIC version of libcpp for the bootloader.
add_library(cpp.pic STATIC ${ALKOS_LIBCPP_SOURCES})
set_target_properties(cpp.pic PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(cpp.pic PUBLIC
    alkos.libcpp.interface
    alkos.target.properties.pic.interface
    gcc
)
target_compile_definitions(cpp.pic PRIVATE __ALKOS_KERNEL_LIBCPP__)

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
        c.pic
        cpp.pic
        gcc
)
