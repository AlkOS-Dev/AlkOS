############################### Error Checking ###############################

alkos_ensure_defined(
    VARS SYSROOT SYSTEM_LIB_TYPE
)

############################### Processing lib type ###############################

if (SYSTEM_LIB_TYPE STREQUAL "k" OR SYSTEM_LIB_TYPE STREQUAL "K")
    set(LIB_NAME "k")
    add_compile_definitions(__ALKOS_LIBK__=1)
elseif (SYSTEM_LIB_TYPE STREQUAL "c" OR SYSTEM_LIB_TYPE STREQUAL "C")
    set(LIB_NAME "c")
else ()
    message(FATAL_ERROR "Unknown SYSTEM_LIB_TYPE : ${SYSTEM_LIB_TYPE}")
endif ()

############################### Configuring files ###############################

alkos_find_sources(LIB_SOURCES)

########################## Configure for given Arch ##########################

add_subdirectory(arch/${ARCH})

############################### Preparing target ###############################

add_library(lib${LIB_NAME} STATIC ${LIB_SOURCES})
set_target_properties(lib${LIB_NAME} PROPERTIES OUTPUT_NAME "${LIB_NAME}")

############################# Setting Custom Properties #########################

target_compile_options(lib${LIB_NAME} PRIVATE
        "$<$<COMPILE_LANGUAGE:CXX>:-mcmodel=kernel>"
        "$<$<COMPILE_LANGUAGE:CXX>:-mno-red-zone>"
        "$<$<COMPILE_LANGUAGE:C>:-mcmodel=kernel>"
        "$<$<COMPILE_LANGUAGE:C>:-mno-red-zone>"
)
target_compile_options(lib${LIB_NAME}.32 PRIVATE
        "$<$<COMPILE_LANGUAGE:CXX>:-mno-red-zone>"
        "$<$<COMPILE_LANGUAGE:C>:-mno-red-zone>"
)


############################### Adding includes ###############################

target_include_directories(lib${LIB_NAME} PUBLIC
        include
)

target_include_directories(lib${LIB_NAME} PRIVATE
        internal
)

target_link_libraries(lib${LIB_NAME} PRIVATE gcc)

target_link_libraries(lib${LIB_NAME} PRIVATE AutoGenLib)

if (SYSTEM_LIB_TYPE STREQUAL "k" OR SYSTEM_LIB_TYPE STREQUAL "K")
    target_link_libraries(lib${LIB_NAME} PRIVATE alkos.kernel.headers)
endif ()

############################### Processing output ###############################

file(MAKE_DIRECTORY ${SYSROOT}/usr/include)
file(MAKE_DIRECTORY ${SYSROOT}/usr/lib)

set_target_properties(lib${LIB_NAME} PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${SYSROOT}/usr/lib
        LIBRARY_OUTPUT_DIRECTORY ${SYSROOT}/usr/lib
)

add_custom_command(TARGET lib${LIB_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/include ${SYSROOT}/usr/include
)
