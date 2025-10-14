message(STATUS "  -> Configuring for x86_64")

################################# Add Modules ##################################

add_subdirectory(boot)

############################### Adding Sources ###############################

set(exclude_patterns ".*/(crti\\.nasm|crtn\\.nasm)$")
if (NOT CMAKE_FEATURE_FLAG_RUN_TEST_MODE)
    list(APPEND exclude_patterns ".*/tests/.*")
endif ()

alkos_target_sources(alkos.kernel
  SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.nasm"
  EXCLUDE ${exclude_patterns}
)

target_include_directories(alkos.kernel PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

################################ Linker Flags ################################

set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/build/kernel.ld")
target_link_options(alkos.kernel PRIVATE
        -T ${LINKER_SCRIPT}
)

########################## CXX Global Constructors ###########################

add_library(alkos.kernel.crti OBJECT
    src/cxx/crti.nasm
)
add_library(alkos.kernel.crtn OBJECT
    src/cxx/crtn.nasm
)

# TODO: Maybe later we can get this code to work
# set_property(TARGET alkos.kernel.config PROPERTY CRTI_OBJ "$<TARGET_OBJECTS:alkos.kernel.crti>")
# set_property(TARGET alkos.kernel.config PROPERTY CRTN_OBJ "$<TARGET_OBJECTS:alkos.kernel.crtn>")

set(CRTI_OBJ_FILE_PATH "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/alkos.kernel.crti.dir/src/cxx/crti.nasm.o")
set(CRTN_OBJ_FILE_PATH "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/alkos.kernel.crtn.dir/src/cxx/crtn.nasm.o")
set_property(TARGET alkos.kernel.config PROPERTY CRTI_OBJ "${CRTI_OBJ_FILE_PATH}")
set_property(TARGET alkos.kernel.config PROPERTY CRTN_OBJ "${CRTN_OBJ_FILE_PATH}")

add_dependencies(alkos.kernel alkos.kernel.crti alkos.kernel.crtn)

################################# Register Env #################################

alkos_register_runtime_environment(
    ARCH_NAME           x86_64
    BOOTABLE_EXECUTABLE alkos.boot.loader.32
    QEMU_COMMAND
        "qemu-system-x86_64"
    QEMU_NORMAL_FLAGS
        "-serial stdio -enable-kvm -cpu max -display default,show-cursor=on -m 4G -smp sockets=1,cores=4,threads=1"
    QEMU_TEST_FLAGS
        "-serial stdio -enable-kvm -cpu max -display none -m 4G -smp sockets=1,cores=4,threads=1"
    MODULES
        alkos.boot.loader.64
        alkos.kernel
    MODULE_COMMANDS
        "loader64"
        "kernel"
)
