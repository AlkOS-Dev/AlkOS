message(STATUS "  -> Configuring for x86_64")

############################### Preparing target ###############################

add_library(c.core.32 STATIC ${LIB_SOURCES})
add_library(c.core.pic STATIC ${LIB_SOURCES})

set_target_properties(c.core.pic PROPERTIES POSITION_INDEPENDENT_CODE ON)

############################### Adding includes ###############################

list(TRANSFORM LIB_PRIVATE_INCLUDE_DIRS PREPEND "../../")
list(TRANSFORM LIB_PUBLIC_INCLUDE_DIRS  PREPEND "../../")

target_include_directories(c.core.32 PUBLIC
  ${LIB_PUBLIC_INCLUDE_DIRS}
)

target_include_directories(c.core.32 PRIVATE
  ${LIB_PRIVATE_INCLUDE_DIRS}
)

target_include_directories(c.core.pic PUBLIC
  ${LIB_PUBLIC_INCLUDE_DIRS}
)

target_include_directories(c.core.pic PRIVATE
  ${LIB_PRIVATE_INCLUDE_DIRS}
)

################################# Dependencies ################################

add_dependencies(c.core.32 c.core)

target_link_libraries(c.core.32 PUBLIC 
  alkos.target.properties.interface.32
  alkos.autogen.interface
  gcc
)

add_dependencies(c.core.pic c.core)

target_link_libraries(c.core.pic PUBLIC 
  alkos.target.properties.pic.interface
  alkos.autogen.interface
  gcc
)
