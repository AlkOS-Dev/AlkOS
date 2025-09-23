#ifndef ALKOS_BOOT_LIB_ABI_TRANSITION_DATA_HPP_
#define ALKOS_BOOT_LIB_ABI_TRANSITION_DATA_HPP_

#include <extensions/defines.hpp>
#include <extensions/types.hpp>

#include "mem/pmm.hpp"
#include "mem/vmm.hpp"

struct alignas(64) TransitionData {
    // TODO: Add magic key for validation
    u64 multiboot_info_addr;
    u64 multiboot_header_start_addr;
    u64 multiboot_header_end_addr;

    alignas(64) PhysicalMemoryManager::PmmState pmm_state;
    alignas(64) VirtualMemoryManager::VmmState vmm_state;
};

struct alignas(64) KernelInitialParams {
    u64 multiboot_info_addr;
    u64 multiboot_header_start_addr;
    u64 multiboot_header_end_addr;
    u64 kernel_start_addr;
    u64 kernel_end_addr;
    u64 pml_4_table_phys_addr;
    u64 mem_info_bitmap_addr;
    u64 mem_info_total_pages;
};

#endif  // ALKOS_BOOT_LIB_ABI_TRANSITION_DATA_HPP_
