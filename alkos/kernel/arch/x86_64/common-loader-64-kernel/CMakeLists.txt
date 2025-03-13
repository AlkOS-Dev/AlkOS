message(STATUS "Configuring common-loader-64-kernel")

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

######################### Setting Custom Properties ##########################

target_compile_options(arch.common.kernel-loader.64 PRIVATE
        "$<$<COMPILE_LANGUAGE:CXX>:-mcmodel=kernel>"
        "$<$<COMPILE_LANGUAGE:CXX>:-mno-red-zone>"
        "$<$<COMPILE_LANGUAGE:C>:-mcmodel=kernel>"
        "$<$<COMPILE_LANGUAGE:C>:-mno-red-zone>"
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
