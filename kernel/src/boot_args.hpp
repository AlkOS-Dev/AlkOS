#ifndef KERNEL_SRC_BOOT_ARGS_HPP_
#define KERNEL_SRC_BOOT_ARGS_HPP_

#include "hal/boot_args.hpp"
#include "mem/types.hpp"

struct FramebufferArgs {
    Mem::PPtr<void> base_address;
    u32 width;
    u32 height;
    u32 pitch;
    u32 bpp;
    u8 type;
};

struct BootArguments {
    Mem::VPtr<void> kernel_start;
    Mem::VPtr<void> kernel_end;
    Mem::PPtr<void> root_page_table;
    Mem::PPtr<void> mem_bitmap;
    size_t total_page_frames;
    Mem::PPtr<void> multiboot_info;
    FramebufferArgs fb_args;
};

BootArguments SanitizeBootArgs(const hal::RawBootArguments &raw_args);

#endif  // KERNEL_SRC_BOOT_ARGS_HPP_
