############################### Error Checking ###############################

if (NOT DEFINED SYSROOT)
    message(FATAL_ERROR "SYSROOT is not defined")
endif ()

if (NOT DEFINED SYSTEM_LIB_TYPE)
    message(FATAL_ERROR "SYSTEM_LIB_TYPE is not defined")
endif ()

############################### Processing lib type ###############################

if (SYSTEM_LIB_TYPE STREQUAL "k" OR SYSTEM_LIB_TYPE STREQUAL "K")
    set(LIB_NAME "k")
    add_compile_definitions(__ALKOS_KERNEL__=1)

    if (USE_DEBUG_OUTPUT)
        message(STATUS "Enabled debug output for libc...")

        # Defining this macro enables debug output
        add_compile_definitions(__USE_DEBUG_OUTPUT__=1)
    endif ()

    if (USE_DEBUG_TRACES)
        message(STATUS "Enabled debug traces for libc...")

        # Controls whether various traces should be printed to debug output
        add_compile_definitions(__USE_DEBUG_TRACES__=1)
    endif ()

elseif (SYSTEM_LIB_TYPE STREQUAL "c" OR SYSTEM_LIB_TYPE STREQUAL "C")
    set(LIB_NAME "c")
else ()
    message(FATAL_ERROR "Unknown SYSTEM_LIB_TYPE : ${SYSTEM_LIB_TYPE}")
endif ()

############################### Configuring files ###############################

file(GLOB_RECURSE LIB_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

########################## Configure for given Arch ##########################

add_subdirectory(arch/${ARCH})

############################### Preparing target ###############################

add_library(lib${LIB_NAME} STATIC ${LIB_SOURCES})
set_target_properties(lib${LIB_NAME} PROPERTIES OUTPUT_NAME "${LIB_NAME}")

############################### Adding includes ###############################

target_include_directories(lib${LIB_NAME} PUBLIC
        include
)

target_link_libraries(lib${LIB_NAME} PRIVATE gcc)

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
