message(STATUS "Configuring x86_64 32-bit loader")

#################################### Exec ####################################

add_executable(alkos.loader32
)

alkos_target_sources(alkos.loader32)

target_include_directories(alkos.loader32 PRIVATE .)

################################### Linking ####################################

target_link_libraries(alkos.loader32 PRIVATE
        alkos.target.properties.interface.32
        alkos.kernel.headers
        arch.common.all.32
        alkos.autogen.interface
        c.core.32
)

############################## Finding CXX Compiler ############################

set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_COMPILER_32} <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

################################ Exec Flags ##################################

set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> <INCLUDES> <FLAGS> -o <OBJECT> <SOURCE>")

################################ Linker Flags ################################

set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/linker.ld")
message(STATUS "32 bit link script: ${LINKER_SCRIPT}")

target_link_options(alkos.loader32 PRIVATE
        -T ${LINKER_SCRIPT}     # Linker script
)

target_compile_definitions(alkos.loader32 PUBLIC
        __i386__=1
)
