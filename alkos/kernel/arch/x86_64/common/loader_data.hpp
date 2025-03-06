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
#include "extensions/types.hpp"

/**
 * @brief Loader data structure
 *
 * This structure is used to pass information from the 32 bit loader to the 64 bit kernel.
 */
struct PACK LoaderData {
    u32 multiboot_info_addr;          // The address of the multiboot info structure
    u32 multiboot_header_start_addr;  // The start address of the multiboot header
    u32 multiboot_header_end_addr;    // The end address of the multiboot header
    u32 loader_start_addr;            // The start address of the loader
    u32 loader_end_addr;              // The end address of the loader
};

#endif  // ALK_OS_KERNEL_ARCH_X86_64_COMMON_LOADER_DATA_HPP_
