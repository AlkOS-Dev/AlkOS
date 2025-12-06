#ifndef KERNEL_SRC_HAL_API_BOOT_ARGS_HPP_
#define KERNEL_SRC_HAL_API_BOOT_ARGS_HPP_

#include <defines.h>
#include <types.hpp>

namespace arch
{
struct RawBootArguments;
struct RawBootArgumentsAPI {
    /// Mem Layout
    u64 kernel_start_addr;
    u64 kernel_end_addr;

    /// VMem
    u64 page_table_phys_addr;

    // Mem Bitmap
    u64 mem_info_bitmap_phys_addr;
    u64 mem_info_total_pages;

    // Framebuffer
    u64 fb_addr;
    u32 fb_width;
    u32 fb_height;
    u32 fb_pitch;
    u32 fb_bpp;

    u8 fb_red_pos;
    u8 fb_red_mask;
    u8 fb_green_pos;
    u8 fb_green_mask;
    u8 fb_blue_pos;
    u8 fb_blue_mask;
};
}  // namespace arch

#endif  // KERNEL_SRC_HAL_API_BOOT_ARGS_HPP_
