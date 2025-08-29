############################### Configuring files ###############################

alkos_find_sources(LIB_SOURCES)
add_library(c.core STATIC ${LIB_SOURCES})

########################## Configure for given Arch ##########################

add_subdirectory(arch/${ARCH})

############################### Adding includes ###############################

target_include_directories(c.core PUBLIC
        include
        abi
)

target_include_directories(c.core PRIVATE
        internal
)

############################### Adding dependencies ###############################

target_link_libraries(c.core PRIVATE 
  alkos.target.properties.interface
  alkos.autogen.interface
  gcc
)
