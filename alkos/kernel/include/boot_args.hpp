#ifndef ALKOS_KERNEL_INCLUDE_BOOT_ARGS_HPP_
#define ALKOS_KERNEL_INCLUDE_BOOT_ARGS_HPP_

#include "hal/boot_args.hpp"
#include "mem/types.hpp"

struct BootArguments {
    Mem::VPtr<void> kernel_start;
    Mem::VPtr<void> kernel_end;
    Mem::PPtr<void> root_page_table;
    Mem::PPtr<void> mem_bitmap;
    size_t total_page_frames;
    Mem::PPtr<void> multiboot_info;
};

BootArguments SanitizeBootArgs(const hal::RawBootArguments raw_args);

#endif  // ALKOS_KERNEL_INCLUDE_BOOT_ARGS_HPP_
