message(STATUS "Configuring x86_64 kernel")

############################## Finding Sources ###############################

file(GLOB_RECURSE ARCH_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.s"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.S"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.asm"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.nasm"
)

# Exclude specific files like crti.nasm and crtn.nasm
list(FILTER ARCH_SOURCES EXCLUDE REGEX ".*/(crti\\.nasm|crtn\\.nasm)$")

if (NOT CMAKE_FEATURE_FLAG_RUN_TEST_MODE)
    list(FILTER ARCH_SOURCES EXCLUDE REGEX ".*/tests/.*")
endif ()

############################### Adding Sources ###############################

target_sources(alkos.kernel PRIVATE ${ARCH_SOURCES})

############################### Adding Headers ###############################

target_include_directories(alkos.kernel.headers INTERFACE
        .
)

################################ Linker Flags ################################

set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/linker.ld")
target_link_options(alkos.kernel PRIVATE
        -T ${LINKER_SCRIPT}     # Linker script
        -nostdlib               # No standard libs
        -z max-page-size=0x1000 # Maximum page size (For 1k alignment)
        -n                      # Omit default mem map
        -lgcc                   # Link against GCC
)

######################### Setting Custom Properties ##########################

target_compile_options(alkos.kernel PRIVATE
        "$<$<COMPILE_LANGUAGE:CXX>:-mcmodel=kernel>"
        "$<$<COMPILE_LANGUAGE:CXX>:-mno-red-zone>"
        "$<$<COMPILE_LANGUAGE:C>:-mcmodel=kernel>"
        "$<$<COMPILE_LANGUAGE:C>:-mno-red-zone>"
)

########################## CXX Global Constructors ###########################

set(SRTI_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cxx/crti.nasm")
set(SRTN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cxx/crtn.nasm")

add_library(alkos.kernel.crts OBJECT
        ${SRTI_FILE}
        ${SRTN_FILE}
)

# Set the CRT object paths
set(CRTI_OBJ "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/alkos.kernel.crts.dir/cxx/crti.nasm.o" PARENT_SCOPE)
set(CRTN_OBJ "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/alkos.kernel.crts.dir/cxx/crtn.nasm.o" PARENT_SCOPE)

add_dependencies(alkos.kernel alkos.kernel.crts)

############################### Link Libraries ###############################

target_link_libraries(alkos.kernel PRIVATE
        arch.common.all.64
        arch.common.kernel-loader.64
        AutoGenLib
)
