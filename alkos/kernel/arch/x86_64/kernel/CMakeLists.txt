message(STATUS "Configuring x86_64 kernel")

############################### Adding Sources ###############################

set(exclude_patterns ".*/(crti\\.nasm|crtn\\.nasm)$")
if (NOT CMAKE_FEATURE_FLAG_RUN_TEST_MODE)
    list(APPEND exclude_patterns ".*/tests/.*")
endif ()

alkos_target_sources(alkos.kernel
    EXCLUDE ${exclude_patterns}
)

############################### Adding Headers ###############################

target_include_directories(alkos.kernel PUBLIC
        .
)

################################ Linker Flags ################################

set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/linker.ld")
target_link_options(alkos.kernel PRIVATE
        -T ${LINKER_SCRIPT}
)

########################## CXX Global Constructors ###########################

add_library(alkos.kernel.crti OBJECT
    cxx/crti.nasm
)
add_library(alkos.kernel.crtn OBJECT
    cxx/crtn.nasm
)

# TODO: Maybe later we can get this code to work
# set_property(TARGET alkos.kernel.config PROPERTY CRTI_OBJ "$<TARGET_OBJECTS:alkos.kernel.crti>")
# set_property(TARGET alkos.kernel.config PROPERTY CRTN_OBJ "$<TARGET_OBJECTS:alkos.kernel.crtn>")
# I used combination of the above code, and target_link_libraries to link in specific order
# Worked but introduced a bug where no tests were detected
# So I reverted to the old way of linking the object files

set(CRTI_OBJ_FILE_PATH "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/alkos.kernel.crti.dir/cxx/crti.nasm.o")
set(CRTN_OBJ_FILE_PATH "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/alkos.kernel.crtn.dir/cxx/crtn.nasm.o")
set_property(TARGET alkos.kernel.config PROPERTY CRTI_OBJ "${CRTI_OBJ_FILE_PATH}")
set_property(TARGET alkos.kernel.config PROPERTY CRTN_OBJ "${CRTN_OBJ_FILE_PATH}")

add_dependencies(alkos.kernel alkos.kernel.crti alkos.kernel.crtn)

############################### Link Libraries ###############################

target_link_libraries(alkos.kernel.deps INTERFACE
        alkos.target.properties.interface
)

# This makes the interface also properly link to object files
target_sources(alkos.kernel.deps INTERFACE
    $<TARGET_OBJECTS:arch.common.all.64>
)
