############################### Preparing target ###############################

add_library(c.core.32 STATIC ${LIB_SOURCES})

############################### Adding includes ###############################

target_include_directories(c.core.32 PUBLIC
        ../../include
)

target_include_directories(c.core.32 PRIVATE
        ../../internal
)

################################# Dependencies ################################

add_dependencies(c.core.32 c.core)

target_link_libraries(c.core.32 PUBLIC 
  alkos.target.properties.interface.32
  alkos.autogen.interface
  gcc
)
