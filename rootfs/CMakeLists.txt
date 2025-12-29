message(STATUS "Configuring Filesystem...")

################################################################################
#                                   Helpers                                    #
################################################################################

include(RuntimeHelpers)

################################################################################
#                                Configuration                                 #
################################################################################

set(ROOTFS_TYPE fat CACHE STRING "Type of root filesystem to create (e.g. fat)")
set(ROOTFS_DIR ${CMAKE_CURRENT_BINARY_DIR}/target CACHE PATH "Staging directory for building root filesystem image")
alkos_add_to_bash_config("CONF_ROOTFS_DIR" "${ROOTFS_DIR}")

if(CMAKE_FEATURE_FLAG_RAMDISK)
    register_filesystem(
            TYPE INITRD
            ROOTFS_TYPE ${ROOTFS_TYPE}
            ROOTFS_TARGET_PATH ${CMAKE_BINARY_DIR}/sysroot/boot/alkos.initrd
            ROOTFS_OVERLAY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/overlay
            MODULE alkos.initrd
            MODULE_COMMAND initrd
    )
else()
    register_filesystem(
            TYPE IMAGE
            ROOTFS_TYPE ${ROOTFS_TYPE}
            ROOTFS_TARGET_PATH ${CMAKE_BINARY_DIR}/alkos.rootfs.img
            ROOTFS_OVERLAY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/overlay
    )
endif()

if(TARGET rootfs AND TARGET userspace_apps)
    add_dependencies(rootfs userspace_apps)
endif()
