message(STATUS "Configuring x86_64 32-bit loader")


#################################### Exec ####################################

add_executable(alkos.loader32
)

############################## Finding Sources ###############################

alkos_target_sources(alkos.loader32)

############################## Finding CXX Compiler ############################

set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_COMPILER_32} <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

############################## Finding Headers ###############################

target_include_directories(alkos.loader32 PRIVATE .)

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

################################### Linking ####################################

target_link_libraries(alkos.loader32 PRIVATE
        target.properties.32
        arch.common.all.32
        libk.32
        alkos.kernel.headers
        AutoGenLib
)
