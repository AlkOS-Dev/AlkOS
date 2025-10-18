#ifndef ALKOS_KERNEL_INCLUDE_BOOT_ARGS_HPP_
#define ALKOS_KERNEL_INCLUDE_BOOT_ARGS_HPP_

#include "hal/boot_args.hpp"
#include "mem/phys/ptr.hpp"
#include "mem/virt/ptr.hpp"

struct BootArguments {
    Mem::VirtualPtr<void> kernel_start;
    Mem::VirtualPtr<void> kernel_end;
    Mem::PhysicalPtr<void> root_page_table;
    Mem::PhysicalPtr<void> mem_bitmap;
    size_t total_page_frames;
    Mem::PhysicalPtr<void> multiboot_info;
};

BootArguments SanitizeBootArgs(const hal::RawBootArguments raw_args);

#endif  // ALKOS_KERNEL_INCLUDE_BOOT_ARGS_HPP_
