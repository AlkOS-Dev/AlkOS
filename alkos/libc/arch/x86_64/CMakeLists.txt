############################### Preparing target ###############################

add_library(lib${LIB_NAME}.32 STATIC ${LIB_SOURCES})
set_target_properties(lib${LIB_NAME}.32 PROPERTIES OUTPUT_NAME "${LIB_NAME}.32")

############################### Adding includes ###############################

target_include_directories(lib${LIB_NAME}.32 PUBLIC
        ../../include
)

target_include_directories(lib${LIB_NAME}.32 PRIVATE
        ../../internal
)

################################# Dependencies ################################

add_dependencies(lib${LIB_NAME}.32 lib${LIB_NAME})

target_link_libraries(lib${LIB_NAME}.32 PUBLIC 
  target.properties.32
  AutoGenLib
  gcc
)

if (SYSTEM_LIB_TYPE STREQUAL "k" OR SYSTEM_LIB_TYPE STREQUAL "K")
    target_link_libraries(lib${LIB_NAME}.32 PRIVATE alkos.kernel.headers)
endif ()

############################### Processing output ###############################

set_target_properties(lib${LIB_NAME}.32 PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${SYSROOT}/usr/lib
        LIBRARY_OUTPUT_DIRECTORY ${SYSROOT}/usr/lib
)
