message(STATUS "Configuring x86_64 common-loader-all")

############################## Preparing targets ###############################

add_library(arch.common.all.64 OBJECT)
add_library(arch.common.all.32 OBJECT)

############################## Finding Sources ###############################

alkos_target_sources(arch.common.all.64)
alkos_target_sources(arch.common.all.32)

############################# Linking libraries ##############################

target_link_libraries(arch.common.all.64 PRIVATE
        alkos.target.properties.interface
        alkos.autogen.interface
        c.core
)

target_link_libraries(arch.common.all.32 PRIVATE
        alkos.target.properties.interface.32
        alkos.kernel.headers
        alkos.autogen.interface
        c.core.32
)

############################### Applying Flags ###############################

set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> <INCLUDES> <FLAGS> -o <OBJECT> <SOURCE>")

############################### Adding Headers ###############################

target_include_directories(alkos.kernel.headers INTERFACE
    .
)
