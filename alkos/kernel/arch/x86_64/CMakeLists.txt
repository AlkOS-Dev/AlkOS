message(STATUS "Configuring for x86_64")

############################# Arch Dependent Vars ############################

set_property(TARGET alkos.kernel.config PROPERTY KERNEL_MODULES "")
set_property(TARGET alkos.kernel.config PROPERTY KERNEL_COMMANDS "")
set_property(TARGET alkos.kernel.config PROPERTY BOOTABLE_KERNEL_EXECUTABLE "")
set_property(TARGET alkos.kernel.config PROPERTY ARCH_QEMU_COMMAND "qemu-system-x86_64")
set_property(TARGET alkos.kernel.config PROPERTY ARCH_QEMU_NORMAL_FLAGS "-serial stdio -enable-kvm -cpu host -display default,show-cursor=on -m 4G -smp sockets=1,cores=4,threads=1")
set_property(TARGET alkos.kernel.config PROPERTY ARCH_QEMU_TEST_FLAGS "-serial stdio -enable-kvm -cpu host -display none -m 4G -smp sockets=1,cores=4,threads=1")

################################### Boot32 ###################################

add_subdirectory(loader32)
set_property(TARGET alkos.kernel.config PROPERTY BOOTABLE_KERNEL_EXECUTABLE alkos.loader32)

################################### Boot64 ###################################

add_subdirectory(loader64)
set_property(TARGET alkos.kernel.config APPEND PROPERTY KERNEL_MODULES alkos.loader64)
set_property(TARGET alkos.kernel.config APPEND PROPERTY KERNEL_COMMANDS "loader64")

################################### Kernel ###################################

add_subdirectory(kernel)
set_property(TARGET alkos.kernel.config APPEND PROPERTY KERNEL_MODULES alkos.kernel)
set_property(TARGET alkos.kernel.config APPEND PROPERTY KERNEL_COMMANDS "kernel")

################################### Common ###################################

add_subdirectory(common-loader-all)
add_subdirectory(common-loader-64-kernel)
