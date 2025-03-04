message(STATUS "Configuring common")

############################## Finding Sources ###############################

file(GLOB_RECURSE COMMON_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

file(GLOB_RECURSE COMMON_ASM
        "${CMAKE_CURRENT_SOURCE_DIR}/*.s"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.S"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.asm"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.nasm"
)

############################## Preparing targets ###############################

add_library(arch.common.kernel-loader.64 OBJECT
        ${COMMON_SOURCES}
        ${COMMON_ASM}
)

############################# Linking libraries ##############################

target_link_libraries(arch.common.kernel-loader.64 PRIVATE
        libk
        arch.common.all.64
)
############################### Applying Flags ###############################

set_source_files_properties(${COMMON_ASM} PROPERTIES COMPILE_FLAGS "-f elf64")

############################### Adding Headers ###############################

target_include_directories(arch.common.kernel-loader.64 PUBLIC
    .
)