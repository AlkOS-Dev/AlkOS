message(STATUS "Configuring x86_64 32-bit loader")

############################## Finding Sources ###############################

file(GLOB_RECURSE ARCH_SOURCES_32
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

file(GLOB_RECURSE ARCH_ASM_32
        "${CMAKE_CURRENT_SOURCE_DIR}/*.s"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.S"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.asm"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.nasm"
)

#################################### Exec ####################################

add_executable(alkos.loader32
        ${ARCH_SOURCES_32}
        ${ARCH_ASM_32}
)

message(STATUS "32 bit compiler: ${CMAKE_CXX_COMPILER_32}")
set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_COMPILER_32} <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

############################### Applying Flags ###############################

target_compile_definitions(alkos.loader32 PRIVATE __USE_DEBUG_OUTPUT__=1)

############################## Finding Headers ###############################

target_include_directories(alkos.loader32 PRIVATE include)
target_include_directories(alkos.loader32 PRIVATE .)

################################ Exec Flags ##################################

set_source_files_properties(${ARCH_SOURCES_32} PROPERTIES COMPILE_FLAGS "-m32")

set_source_files_properties(${ARCH_ASM_32} PROPERTIES COMPILE_FLAGS "-f elf32 ")

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
)