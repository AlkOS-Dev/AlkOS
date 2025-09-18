#ifndef ALKOS_KERNEL_ARCH_X86_64_SRC_ABI_BOOT_PARAMS_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_SRC_ABI_BOOT_PARAMS_HPP_

#include <extensions/defines.hpp>
#include <extensions/types.hpp>

struct alignas(64) KernelInitialParams {
    u64 multiboot_info_addr;
    u64 multiboot_header_start_addr;
    u64 multiboot_header_end_addr;
    u64 kernel_start_addr;
    u64 kernel_end_addr;
    u64 pml_4_table_phys_addr;
    u64 mem_info_bitmap_addr;
};

#endif  // ALKOS_KERNEL_ARCH_X86_64_SRC_ABI_BOOT_PARAMS_HPP_
