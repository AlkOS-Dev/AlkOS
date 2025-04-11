#ifndef ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_DEFINITIONS_LOADER64_DATA_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_DEFINITIONS_LOADER64_DATA_HPP_

namespace loader64
{
/// This is the structure that is passed from the 64 bit loader to the kernel
struct PACK LoaderData {
    u64 multiboot_info_addr;          ///< The address of the multiboot info structure
    u64 multiboot_header_start_addr;  ///< The start address of the multiboot header
    u64 multiboot_header_end_addr;    ///< The end address of the multiboot header
    u64 loader_memory_manager_addr;   ///< The address of the loader memory manager
    u64 kernel_start_addr;            ///< The start address of the kernel
    u64 kernel_end_addr;              ///< The end address of the kernel
};
}  // namespace loader64

#endif  // ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_DEFINITIONS_LOADER64_DATA_HPP_
