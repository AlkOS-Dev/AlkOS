#ifndef ALK_OS_KERNEL_ARCH_X86_64_COMMON_LOADER_DATA_HPP_
#define ALK_OS_KERNEL_ARCH_X86_64_COMMON_LOADER_DATA_HPP_

/**
 * loader_data.h
 * This file contains the definition of the LoaderData structure.
 * This structure is used to pass information from the 32 bit loader to the 64 bit kernel.
 * It is shared between the two. The 32 bit loader should fill this structure with the necessary
 * information
 */

#include <defines.hpp>
#include <types.hpp>

/**
 * @brief Loader data structure
 *
 * This structure is used to pass information from the 32 bit loader to the 64 bit loader.
 */
struct PACK LoaderData_32_64_Pass {
    u32 multiboot_info_addr;          // The address of the multiboot info structure
    u32 multiboot_header_start_addr;  // The start address of the multiboot header
    u32 multiboot_header_end_addr;    // The end address of the multiboot header
    u32 loader_start_addr;            // The start address of the loader
    u32 loader_end_addr;              // The end address of the loader
    u64 loader_memory_manager_addr;   // The address of the loader memory manager
};

/// This is the structure that is passed from the 64 bit loader to the kernel
/// It is unused for now but will be used in physical memory management update
struct PACK LoaderDataKernelPass {
    u64 dummy;
};

#endif  // ALK_OS_KERNEL_ARCH_X86_64_COMMON_LOADER_DATA_HPP_
