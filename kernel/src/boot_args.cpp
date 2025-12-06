
#include <assert.h>

#include "boot_args.hpp"
#include "hal/boot_args.hpp"
#include "mem/types.hpp"
#include "trace_framework.hpp"

using namespace Mem;

BootArguments SanitizeBootArgs(const hal::RawBootArguments &raw_args)
{
    R_ASSERT_NOT_ZERO(
        raw_args.multiboot_info_phys_addr, "Multiboot info physical address is null."
    );
    R_ASSERT_NOT_ZERO(raw_args.kernel_start_addr, "Kernel start physical address is null.");
    R_ASSERT_NOT_ZERO(raw_args.kernel_end_addr, "Kernel end physical address is null.");
    R_ASSERT_NOT_ZERO(
        raw_args.mem_info_bitmap_phys_addr, "Memory bitmap physical address is null."
    );
    R_ASSERT_GT(raw_args.mem_info_total_pages, 0UL, "Total memory pages is zero.");

    FramebufferArgs fb_args{
        .base_address = UptrToPtr<void>(raw_args.fb_addr),
        .width        = raw_args.fb_width,
        .height       = raw_args.fb_height,
        .pitch        = raw_args.fb_pitch,
        .bpp          = raw_args.fb_bpp,
        .red_pos      = raw_args.fb_red_pos,
        .red_mask     = raw_args.fb_red_mask,
        .green_pos    = raw_args.fb_green_pos,
        .green_mask   = raw_args.fb_green_mask,
        .blue_pos     = raw_args.fb_blue_pos,
        .blue_mask    = raw_args.fb_blue_mask,
    };

    BootArguments sanitized_k_args{
        .kernel_start      = UptrToPtr<void>(raw_args.kernel_start_addr),
        .kernel_end        = UptrToPtr<void>(raw_args.kernel_end_addr),
        .root_page_table   = UptrToPtr<void>(raw_args.page_table_phys_addr),
        .mem_bitmap        = UptrToPtr<void>(raw_args.mem_info_bitmap_phys_addr),
        .total_page_frames = static_cast<size_t>(raw_args.mem_info_total_pages),
        .multiboot_info    = UptrToPtr<void>(raw_args.multiboot_info_phys_addr),
        .fb_args           = fb_args,
    };

    DEBUG_INFO_BOOT("Sanitized boot arguments:");
    DEBUG_INFO_BOOT(
        "  Boot Arguments:\n"
        "    kernel_start:       %p\n"
        "    kernel_end:         %p\n"
        "    root_page_table:    %p\n"
        "    mem_bitmap:         %p\n"
        "    total_page_frames:  %zu\n"
        "    multiboot_info:     %p\n"
        "  Framebuffer Arguments:\n"
        "    base_address:       %p\n"
        "    width:              %u\n"
        "    height:             %u\n"
        "    pitch:              %u\n"
        "    bpp:                %u\n"
        "    red_pos:            %hhu\n"
        "    red_mask:           %hhu\n"
        "    green_pos:          %hhu\n"
        "    green_mask:         %hhu\n"
        "    blue_pos:           %hhu\n"
        "    blue_mask:          %hhu\n",
        sanitized_k_args.kernel_start, sanitized_k_args.kernel_end,
        sanitized_k_args.root_page_table, sanitized_k_args.mem_bitmap,
        sanitized_k_args.total_page_frames, sanitized_k_args.multiboot_info,
        sanitized_k_args.fb_args.base_address, sanitized_k_args.fb_args.width,
        sanitized_k_args.fb_args.height, sanitized_k_args.fb_args.pitch,
        sanitized_k_args.fb_args.bpp, sanitized_k_args.fb_args.red_pos,
        sanitized_k_args.fb_args.red_mask, sanitized_k_args.fb_args.green_pos,
        sanitized_k_args.fb_args.green_mask, sanitized_k_args.fb_args.blue_pos,
        sanitized_k_args.fb_args.blue_mask
    );

    return sanitized_k_args;
}
