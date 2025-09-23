############################### Preparing target ###############################

add_library(c.core.32 STATIC ${LIB_SOURCES})

############################### Adding includes ###############################

list(TRANSFORM LIB_PRIVATE_INCLUDE_DIRS PREPEND "../../")
list(TRANSFORM LIB_PUBLIC_INCLUDE_DIRS  PREPEND "../../")

target_include_directories(c.core.32 PUBLIC
  ${LIB_PUBLIC_INCLUDE_DIRS}
)

target_include_directories(c.core.32 PRIVATE
  ${LIB_PRIVATE_INCLUDE_DIRS}
)

################################# Dependencies ################################

add_dependencies(c.core.32 c.core)

target_link_libraries(c.core.32 PUBLIC 
  alkos.target.properties.interface.32
  alkos.autogen.interface
  gcc
)
