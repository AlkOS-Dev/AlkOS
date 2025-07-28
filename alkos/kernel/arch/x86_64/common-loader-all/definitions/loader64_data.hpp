#ifndef ALKOS_ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_64_KERNEL_DEFINITIONS_LOADER64_DATA_HPP_
#define ALKOS_ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_64_KERNEL_DEFINITIONS_LOADER64_DATA_HPP_

/// This is the structure that is passed from the 64 bit loader to the kernel
namespace loader64
{
struct PACK LoaderData {
    u64 multiboot_info_addr;          // The address of the multiboot info structure
    u64 multiboot_header_start_addr;  // The start address of the multiboot header
    u64 multiboot_header_end_addr;    // The end address of the multiboot header
    u64 loader_memory_manager_addr;   // The address of the loader memory manager
    u64 kernel_start_addr;            // The start address of the kernel
    u64 kernel_end_addr;              // The end address of the kernel
};
}  // namespace loader64

#endif  // ALKOS_ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_64_KERNEL_DEFINITIONS_LOADER64_DATA_HPP_
