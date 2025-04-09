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

if (NOT BOOTABLE_KERNEL_EXECUTABLE)
    message(FATAL_ERROR "No primary kernel executable defined")
endif ()

message(STATUS "Bootable kernel executable: ${BOOTABLE_KERNEL_EXECUTABLE}")
message(STATUS "Kernel modules: ${KERNEL_MODULES}")

message(STATUS "CRTI_OBJ: ${CRTI_OBJ}, CRTN_OBJ: ${CRTN_OBJ}")

########################## Back-Propagate Variables ##########################

set(CRTI_OBJ "${CRTI_OBJ}" PARENT_SCOPE)
set(CRTN_OBJ "${CRTN_OBJ}" PARENT_SCOPE)

set(BOOTABLE_KERNEL_EXECUTABLE ${BOOTABLE_KERNEL_EXECUTABLE} PARENT_SCOPE)
set(KERNEL_MODULES ${KERNEL_MODULES} PARENT_SCOPE)
set(KERNEL_COMMANDS ${KERNEL_COMMANDS} PARENT_SCOPE)

set(ARCH_QEMU_COMMAND ${ARCH_QEMU_COMMAND} PARENT_SCOPE)
set(ARCH_QEMU_NORMAL_FLAGS ${ARCH_QEMU_NORMAL_FLAGS} PARENT_SCOPE)
set(ARCH_QEMU_TEST_FLAGS ${ARCH_QEMU_TEST_FLAGS} PARENT_SCOPE)
