message(STATUS "Configuring x86_64 64-bit loader")

############################## Finding Sources ###############################

file(GLOB_RECURSE ARCH_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.s"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.S"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.asm"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.nasm"
)

#################################### Exec ####################################

add_executable(alkos.loader64
        ${ARCH_SOURCES}
)

############################### Adding Headers ###############################

target_include_directories(alkos.loader64 PRIVATE
        .
)

################################ Linker Flags ################################

set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/linker.ld")
target_link_options(alkos.loader64 PRIVATE
        -T ${LINKER_SCRIPT}     # Linker script
        -nostdlib               # No standard libs
        -z max-page-size=0x1000 # Maximum page size (For 1k alignment)
        -n                      # Omit default mem map
        -lgcc                   # Link against GCC
)

############################### Link Libraries ###############################

target_link_libraries(alkos.loader64 PRIVATE
        gcc
        libk
        arch.common.all.64
        arch.common.kernel-loader.64
        alkos.kernel.headers
        AutoGenLib
)
