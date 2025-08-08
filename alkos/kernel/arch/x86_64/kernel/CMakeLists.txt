message(STATUS "Configuring x86_64 kernel")

############################### Adding Sources ###############################

# Define patterns to always exclude
set(exclude_patterns ".*/(crti\\.nasm|crtn\\.nasm)$")

# Conditionally exclude test files
if (NOT CMAKE_FEATURE_FLAG_RUN_TEST_MODE)
    list(APPEND exclude_patterns ".*/tests/.*")
endif ()

alkos_target_sources(alkos.kernel
    EXCLUDE ${exclude_patterns}
)

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

target_link_libraries(alkos.kernel PRIVATE
        target.properties
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
