message(STATUS "Configuring for x86_64")

################################# Add Modules ##################################

add_subdirectory(loader32)
add_subdirectory(loader64)
add_subdirectory(kernel)

add_subdirectory(common-loader-all)
add_subdirectory(common-loader-64-kernel)

################################# Register Env #################################

alkos_register_runtime_environment(
    ARCH_NAME           x86_64
    BOOTABLE_EXECUTABLE alkos.loader32
    QEMU_COMMAND
        "qemu-system-x86_64"
    QEMU_NORMAL_FLAGS
        "-serial stdio -enable-kvm -cpu max-display default,show-cursor=on -m 4G -smp sockets=1,cores=4,threads=1"
    QEMU_TEST_FLAGS
        "-serial stdio -enable-kvm -cpu max -display none -m 4G -smp sockets=1,cores=4,threads=1"
    MODULES
        alkos.loader64
        alkos.kernel
    MODULE_COMMANDS
        "loader64"
        "kernel"
)
