message(STATUS "Configuring for x86_64")

############################# Arch Dependent Vars ############################

set(KERNEL_MODULES "")
set(KERNEL_COMMANDS "") # Kernel commands for the bootloader, has to be same size as KERNEL_MODULES
set(BOOTABLE_KERNEL_EXECUTABLE "")
set(ARCH_QEMU_COMMAND "qemu-system-x86_64")
set(ARCH_QEMU_NORMAL_FLAGS "-serial stdio -enable-kvm -cpu host -display default,show-cursor=on -m 4G -smp sockets=1,cores=4,threads=1")
set(ARCH_QEMU_TEST_FLAGS "-serial stdio -enable-kvm -cpu host -display none -m 4G -smp sockets=1,cores=4,threads=1")

################################### Boot32 ###################################

add_subdirectory(loader32)
# Retrieve the bootable kernel executable
set(BOOTABLE_KERNEL_EXECUTABLE alkos.loader32)

################################### Boot64 ###################################

add_subdirectory(loader64)
set(KERNEL_MODULES ${KERNEL_MODULES} alkos.loader64)
set(KERNEL_COMMANDS ${KERNEL_COMMANDS} "loader64")

################################### Kernel ###################################

add_subdirectory(kernel)
set(KERNEL_MODULES ${KERNEL_MODULES} alkos.kernel)
set(KERNEL_COMMANDS ${KERNEL_COMMANDS} "kernel")

################################### Common ###################################

add_subdirectory(common-loader-all)
add_subdirectory(common-loader-64-kernel)

############################### Error Checking ###############################

alkos_ensure_defined(
    VARS BOOTABLE_KERNEL_EXECUTABLE
    MESSAGE "No primary kernel executable defined for x86_64 architecture."
)

message(STATUS "Bootable kernel executable: ${BOOTABLE_KERNEL_EXECUTABLE}")
message(STATUS "Kernel modules: ${KERNEL_MODULES}")

########################## Set Kernel Properties ##########################

set_property(TARGET alkos.kernel.config PROPERTY BOOTABLE_KERNEL_EXECUTABLE ${BOOTABLE_KERNEL_EXECUTABLE})
set_property(TARGET alkos.kernel.config PROPERTY KERNEL_MODULES ${KERNEL_MODULES})
set_property(TARGET alkos.kernel.config PROPERTY KERNEL_COMMANDS ${KERNEL_COMMANDS})

set_property(TARGET alkos.kernel.config PROPERTY ARCH_QEMU_COMMAND ${ARCH_QEMU_COMMAND})
set_property(TARGET alkos.kernel.config PROPERTY ARCH_QEMU_NORMAL_FLAGS ${ARCH_QEMU_NORMAL_FLAGS})
set_property(TARGET alkos.kernel.config PROPERTY ARCH_QEMU_TEST_FLAGS ${ARCH_QEMU_TEST_FLAGS})
