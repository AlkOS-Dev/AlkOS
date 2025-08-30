message(STATUS "Configuring 32-bit bootloader")

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
        c.core.32
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

