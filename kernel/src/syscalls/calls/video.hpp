#ifndef KERNEL_SRC_SYSCALLS_CALLS_VIDEO_HPP_
#define KERNEL_SRC_SYSCALLS_CALLS_VIDEO_HPP_

#include "hardware/core_local.hpp"
#include "modules/video.hpp"
#include "trace_framework.hpp"

#include <geometry.hpp>

/**
 * @brief Registers the current process as a graphics app, allocates a buffer,
 * maps it to userspace, and fills the info struct.
 */
FORCE_INLINE_F void SysCreateGraphicSession(GuiBufferInfo *user_info)
{
    if (user_info == nullptr) {
        return;
    }

    auto &wm = VideoModule::Get().GetWindowManager();
    auto &fb = VideoModule::Get().GetFramebuffer();

    // Create Session (Allocates PMM, Maps to VMM)
    auto result = wm.CreateSession();

    if (result) {
        // Fill userspace struct
        user_info->buffer_ptr = *result;

        const auto fb_info = fb.GetInfo();
        user_info->width   = fb_info.width;
        user_info->height  = fb_info.height;
        user_info->pitch   = fb_info.pitch;
        user_info->bpp     = 32;

        user_info->format.red_pos         = fb_info.format.red_pos;
        user_info->format.red_mask_size   = fb_info.format.red_mask_size;
        user_info->format.green_pos       = fb_info.format.green_pos;
        user_info->format.green_mask_size = fb_info.format.green_mask_size;
        user_info->format.blue_pos        = fb_info.format.blue_pos;
        user_info->format.blue_mask_size  = fb_info.format.blue_mask_size;
    } else {
        user_info->buffer_ptr = nullptr;
    }
}

/**
 * @brief Copies the current process's backbuffer to the physical screen
 */
FORCE_INLINE_F void SysBlit()
{
    auto &wm = VideoModule::Get().GetWindowManager();
    auto pid = hardware::GetRunningPid();

    DEBUG_INFO_GENERAL("SysBlit called by PID: %llu", pid);
    wm.Blit(pid);
}

#endif  // KERNEL_SRC_SYSCALLS_CALLS_VIDEO_HPP_
