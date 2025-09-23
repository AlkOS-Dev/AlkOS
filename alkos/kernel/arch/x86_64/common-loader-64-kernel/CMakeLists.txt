message(STATUS "Configuring common-loader-64-kernel")

############################## Preparing targets ###############################

add_library(arch.common.kernel-loader.64 OBJECT)

alkos_target_sources(arch.common.kernel-loader.64)

target_include_directories(alkos.kernel.headers INTERFACE
    .
)

############################# Linking libraries ##############################

target_link_libraries(arch.common.kernel-loader.64 PRIVATE
        alkos.target.properties.interface
        arch.common.all.64
        alkos.autogen.interface
        c.core
)
