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

add_library(alkos.kernel.crti OBJECT
    cxx/crti.nasm
)
add_library(alkos.kernel.crtn OBJECT
    cxx/crtn.nasm
)

############################### Link Libraries ###############################

target_link_libraries(alkos.kernel.deps INTERFACE
        arch.common.all.64
        arch.common.kernel-loader.64
)

# This makes the interface also properly link to object files
target_sources(alkos.kernel.deps INTERFACE
    $<TARGET_OBJECTS:arch.common.all.64>
    $<TARGET_OBJECTS:arch.common.kernel-loader.64>
)
