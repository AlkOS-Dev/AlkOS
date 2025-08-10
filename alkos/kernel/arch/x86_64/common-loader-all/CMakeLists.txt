message(STATUS "Configuring x86_64 common-loader-all")

############################## Preparing targets ###############################

add_library(arch.common.all.64 OBJECT)
add_library(arch.common.all.32 OBJECT)

############################## Finding Sources ###############################

alkos_target_sources(arch.common.all.64)
alkos_target_sources(arch.common.all.32)

############################# Linking libraries ##############################

target_link_libraries(arch.common.all.64 PRIVATE
        target.properties
        libk
        alkos.kernel.headers
        AutoGenLib
)

target_link_libraries(arch.common.all.32 PRIVATE
        target.properties.32
        libk.32
        alkos.kernel.headers
        AutoGenLib
)

############################### Applying Flags ###############################

set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> <INCLUDES> <FLAGS> -o <OBJECT> <SOURCE>")

############################### Adding Headers ###############################

target_include_directories(alkos.kernel.headers INTERFACE
    .
)
