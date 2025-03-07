############################### Preparing target ###############################

add_library(lib${LIB_NAME}.32 STATIC ${LIB_SOURCES})
set_target_properties(lib${LIB_NAME}.32 PROPERTIES OUTPUT_NAME "${LIB_NAME}.32")

target_compile_options(lib${LIB_NAME}.32 PRIVATE -m32)

############################### Adding includes ###############################

target_include_directories(lib${LIB_NAME}.32 PUBLIC
        ../../include
)

################################# Dependencies ################################

add_dependencies(lib${LIB_NAME}.32 lib${LIB_NAME})

############################### Processing output ###############################

set_target_properties(lib${LIB_NAME}.32 PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${SYSROOT}/usr/lib
        LIBRARY_OUTPUT_DIRECTORY ${SYSROOT}/usr/lib
)
target_link_libraries(lib${LIB_NAME}.32 PRIVATE gcc)
