#ifndef ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_DEFINITIONS_LOADER32_DATA_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_DEFINITIONS_LOADER32_DATA_HPP_

#include <extensions/defines.hpp>
#include "extensions/types.hpp"

namespace loader32
{

/// This structure is used to pass information from the 32 bit loader to the 64 bit loader.
struct PACK LoaderData {
    u64 multiboot_info_addr;          ///< The address of the multiboot info structure
    u64 multiboot_header_start_addr;  ///< The start address of the multiboot header
    u64 multiboot_header_end_addr;    ///< The end address of the multiboot header
    u64 loader_memory_manager_addr;   ///< The address of the loader memory manager
};

}  // namespace loader32

#endif  // ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_DEFINITIONS_LOADER32_DATA_HPP_
