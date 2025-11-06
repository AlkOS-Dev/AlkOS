message(STATUS "Configuring LibC...")

############################### Configuring files ###############################

alkos_find_sources(LIB_SOURCES)

add_library(c.core STATIC ${LIB_SOURCES})

set(LIB_PUBLIC_INCLUDE_DIRS
  include
  abi
)

set(LIB_PRIVATE_INCLUDE_DIRS
  internal
)

########################## Configure for given Arch ##########################

add_subdirectory(arch/${ARCH})

############################### Adding includes ###############################


target_include_directories(c.core PUBLIC
  ${LIB_PUBLIC_INCLUDE_DIRS}
)

target_include_directories(c.core PRIVATE
  ${LIB_PRIVATE_INCLUDE_DIRS}
)

############################### Adding dependencies ###############################

target_link_libraries(c.core PRIVATE 
  alkos.target.properties.interface
  alkos.autogen.interface
  gcc
)
