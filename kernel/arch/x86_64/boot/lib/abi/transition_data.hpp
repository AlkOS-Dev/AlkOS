#ifndef KERNEL_ARCH_X86_64_BOOT_LIB_ABI_TRANSITION_DATA_HPP_
#define KERNEL_ARCH_X86_64_BOOT_LIB_ABI_TRANSITION_DATA_HPP_

#include <defines.hpp>
#include <types.hpp>

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

struct PACK alignas(64) KernelArguments {
    /// Kernel Mem Layout
    u64 kernel_start_addr;
    u64 kernel_end_addr;

    /// VMem
    u64 pml_4_table_phys_addr;

    /// Memory Bitmap
    u64 mem_info_bitmap_addr;
    u64 mem_info_total_pages;

    /// Multiboot
    u64 multiboot_info_addr;
    u64 multiboot_header_start_addr;
    u64 multiboot_header_end_addr;

    /// Framebuffer
    u64 fb_addr;
    u32 fb_width;
    u32 fb_height;
    u32 fb_pitch;
    u32 fb_bpp;  // Bits per pixel

    // RGB Format
    u8 fb_red_pos;
    u8 fb_red_mask;
    u8 fb_green_pos;
    u8 fb_green_mask;
    u8 fb_blue_pos;
    u8 fb_blue_mask;
};

#endif  // KERNEL_ARCH_X86_64_BOOT_LIB_ABI_TRANSITION_DATA_HPP_
