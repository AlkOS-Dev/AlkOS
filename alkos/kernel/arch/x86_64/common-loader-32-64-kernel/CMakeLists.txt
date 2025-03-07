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

file(GLOB_RECURSE COMMON_ASM_32
        "${CMAKE_CURRENT_SOURCE_DIR}/*.s"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.S"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.asm"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.nasm"
)

############################## Preparing targets ###############################

add_library(arch.common.all.64 OBJECT
        ${COMMON_SOURCES}
        ${COMMON_ASM}
)

add_library(arch.common.all.32 OBJECT
        ${COMMON_SOURCES}
        ${COMMON_ASM_32}
)

############################# Linking libraries ##############################

target_link_libraries(arch.common.all.64 PRIVATE
        libk
)

target_link_libraries(arch.common.all.32 PRIVATE
        libk.32
)

######################### Setting Custom Properties ##########################

target_compile_options(arch.common.all.64 PRIVATE
        "$<$<COMPILE_LANGUAGE:CXX>:-mcmodel=kernel>"
        "$<$<COMPILE_LANGUAGE:CXX>:-mno-red-zone>"
        "$<$<COMPILE_LANGUAGE:C>:-mcmodel=kernel>"
        "$<$<COMPILE_LANGUAGE:C>:-mno-red-zone>"
)
target_compile_options(arch.common.all.32 PRIVATE
        "$<$<COMPILE_LANGUAGE:CXX>:-mno-red-zone>"
        "$<$<COMPILE_LANGUAGE:C>:-mno-red-zone>"
)


############################### Applying Flags ###############################

target_compile_options(arch.common.all.32 PRIVATE "-m32")

set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> <INCLUDES> <FLAGS> -o <OBJECT> <SOURCE>")

set_source_files_properties(${COMMON_ASM_32} PROPERTIES COMPILE_FLAGS "-f elf32")
set_source_files_properties(${COMMON_ASM} PROPERTIES COMPILE_FLAGS "-f elf64")

############################### Adding Headers ###############################

target_include_directories(arch.common.all.64 PUBLIC
    .
)

target_include_directories(arch.common.all.32 PUBLIC
    .
)
