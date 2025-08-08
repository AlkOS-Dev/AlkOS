message(STATUS "Configuring x86_64 32-bit loader")


#################################### Exec ####################################

add_executable(alkos.loader32
)

############################## Finding Sources ###############################

alkos_target_sources(alkos.loader32)

############################## Finding CXX Compiler ############################

message(STATUS "32 bit compiler: ${CMAKE_CXX_COMPILER_32}")
set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_COMPILER_32} <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

############################## Finding Headers ###############################

target_include_directories(alkos.loader32 PRIVATE .)

################################ Exec Flags ##################################

target_link_libraries(alkos.loader32 PRIVATE target.properties.32)

set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> <INCLUDES> <FLAGS> -o <OBJECT> <SOURCE>")

################################ Linker Flags ################################

set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/linker.ld")
message(STATUS "32 bit link script: ${LINKER_SCRIPT}")

target_link_options(alkos.loader32 PRIVATE
        -T ${LINKER_SCRIPT}     # Linker script
        -nostdlib               # No standard libs
        -z max-page-size=0x1000 # Maximum page size (For 1k alignment)
        -n                      # Omit default mem map
        -lgcc                   # Link against GCC
)

target_compile_definitions(alkos.loader32 PUBLIC
        __i386__=1
)

target_link_libraries(alkos.loader32 PRIVATE
        gcc
        arch.common.all.32
        libk.32
        alkos.kernel.headers
        AutoGenLib
)
