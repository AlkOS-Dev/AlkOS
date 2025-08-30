#ifndef ALKOS_BOOT_LIB_ABI_TRANSITION_DATA_HPP_
#define ALKOS_BOOT_LIB_ABI_TRANSITION_DATA_HPP_

#include <extensions/defines.hpp>
#include <extensions/types.hpp>

struct alignas(64) TransitionData {
    u64 multiboot_info_addr;
    u64 multiboot_header_start_addr;
    u64 multiboot_header_end_addr;
    u64 loader_memory_manager_addr;
};

struct alignas(64) KernelInitialParams {
    u64 multiboot_info_addr;
    u64 multiboot_header_start_addr;
    u64 multiboot_header_end_addr;
    u64 loader_memory_manager_addr;
    u64 kernel_start_addr;
    u64 kernel_end_addr;
};

#endif  // ALKOS_BOOT_LIB_ABI_TRANSITION_DATA_HPP_
