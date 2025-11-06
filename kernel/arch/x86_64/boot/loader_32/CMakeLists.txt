message(STATUS "      -> Configuring 32-bit bootloader")

############################# Libc Instantiation #############################

# Create a 32-bit version of libc for the bootloader.
add_library(c.32 STATIC ${ALKOS_LIBC_SOURCES})
target_link_libraries(c.32 PUBLIC
    alkos.libc.interface
    alkos.target.properties.interface.32
    gcc
)
target_compile_definitions(c.32 PRIVATE __ALKOS_KERNEL_LIBC__)

# Create a 32-bit version of libcpp for the bootloader.
add_library(cpp.32 STATIC ${ALKOS_LIBCPP_SOURCES})
target_link_libraries(cpp.32 PUBLIC
    alkos.libcpp.interface
    alkos.target.properties.interface.32
    gcc
)
target_compile_definitions(cpp.32 PRIVATE __ALKOS_KERNEL_LIBCPP__)

#################################### Exec ####################################

add_executable(alkos.boot.loader.32)

alkos_target_sources(alkos.boot.loader.32
    SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.nasm"
        "${CMAKE_CURRENT_SOURCE_DIR}/build/multiboot_header.nasm"
)

target_include_directories(alkos.boot.loader.32 PRIVATE .)

################################### Linking ####################################

target_link_libraries(alkos.boot.loader.32 PRIVATE
        alkos.target.properties.interface.32
        alkos.autogen.interface
        alkos.boot.lib.interface
        c.32
        cpp.32
)

############################## Compile Definitions #############################

target_compile_definitions(alkos.boot.loader.32 PUBLIC
        __i386__=1
)

############################## Finding CXX Compiler ############################

set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_COMPILER_32} <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

################################ Exec Flags ##################################

set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> <INCLUDES> <FLAGS> -o <OBJECT> <SOURCE>")

################################ Linker Flags ################################

set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/build/linker.ld")
target_link_options(alkos.boot.loader.32 PRIVATE
        -T ${LINKER_SCRIPT}     
)
