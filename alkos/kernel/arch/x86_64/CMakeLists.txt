message(STATUS "Configuring for x86_64")

############################# Arch Dependent Vars ############################

set(KERNEL_MODULES "")
set(BOOTABLE_KERNEL_EXECUTABLE "")

################################### Boot32 ###################################

add_subdirectory(loader32)
set_target_properties(alkos.loader32 PROPERTIES
        IMPORTED_LOCATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/alkos.loader32"
)

# Retrieve the bootable kernel executable
set(BOOTABLE_KERNEL_EXECUTABLE alkos.loader32)

################################### Boot64 ###################################

add_subdirectory(loader64)
set(KERNEL_MODULES ${KERNEL_MODULES} alkos.loader64)

################################### Kernel ###################################

add_subdirectory(kernel)
set(KERNEL_MODULES ${KERNEL_MODULES} alkos.kernel)

################################### Common ###################################

add_subdirectory(common-loader-32-64-kernel)
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
