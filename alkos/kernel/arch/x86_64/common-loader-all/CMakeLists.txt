message(STATUS "Configuring x86_64 common-loader-all")

############################## Preparing targets ###############################

add_library(arch.common.all.64 OBJECT)
add_library(arch.common.all.32 OBJECT)

############################## Finding Sources ###############################

alkos_target_sources(arch.common.all.64)
alkos_target_sources(arch.common.all.32)

############################# Linking libraries ##############################

target_link_libraries(arch.common.all.64 PRIVATE
        libk
        target.properties
)

target_link_libraries(arch.common.all.32 PRIVATE
        libk.32
        target.properties.32
)

############################### Applying Flags ###############################

target_compile_options(arch.common.all.32 PRIVATE "-m32")

set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> <INCLUDES> <FLAGS> -o <OBJECT> <SOURCE>")

set_source_files_properties(${COMMON_ASM_32} PROPERTIES COMPILE_FLAGS "-f elf32")
set_source_files_properties(${COMMON_ASM} PROPERTIES COMPILE_FLAGS "-f elf64")

############################### Adding Headers ###############################

target_include_directories(alkos.kernel.headers INTERFACE
    .
)

target_link_libraries(arch.common.all.64 PRIVATE
        alkos.kernel.headers
        AutoGenLib
)

target_link_libraries(arch.common.all.32 PRIVATE
        alkos.kernel.headers
        AutoGenLib
)
