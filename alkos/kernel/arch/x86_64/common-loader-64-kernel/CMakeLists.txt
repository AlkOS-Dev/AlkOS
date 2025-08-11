message(STATUS "Configuring common-loader-64-kernel")

############################## Preparing targets ###############################

add_library(arch.common.kernel-loader.64 OBJECT
)
############################## Finding Sources ###############################

alkos_target_sources(arch.common.kernel-loader.64)

######################### Setting Custom Properties ##########################

target_link_libraries(arch.common.kernel-loader.64 PRIVATE
        target.properties
)

############################# Linking libraries ##############################

target_link_libraries(arch.common.kernel-loader.64 PRIVATE
        arch.common.all.64
        alkos.kernel.headers
        libk
        AutoGenLib
)

############################### Adding Headers ###############################

target_include_directories(alkos.kernel.headers INTERFACE
    .
)
