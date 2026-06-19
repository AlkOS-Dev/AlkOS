// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef LIBS_LIBC_SRC_INCLUDE_ALKOS_SYS_VIDEO_H_
#define LIBS_LIBC_SRC_INCLUDE_ALKOS_SYS_VIDEO_H_

#include "alkos/video.h"
#include "defines.h"
#include "platform.h"
#include "types.h"

BEGIN_DECL_C

FORCE_INLINE_F void Blit() { __platform_blit(); }

FORCE_INLINE_F GuiBufferInfo GetVideoBufferInfo()
{
    GuiBufferInfo info;
    __platform_create_graphic_session(&info);
    return info;
}

END_DECL_C

#ifdef __cplusplus

#include <tuple.hpp>

FORCE_INLINE_F std::tuple<Graphics::Surface, Graphics::PixelFormat> GetVideoContext()
{
    GuiBufferInfo info = GetVideoBufferInfo();

    auto surface = Graphics::Surface(
        static_cast<Graphics::NativePixel *>(info.buffer_ptr), info.width, info.height, info.pitch
    );

    Graphics::PixelFormat format{info.format.red_pos,   info.format.red_mask_size,
                                 info.format.green_pos, info.format.green_mask_size,
                                 info.format.blue_pos,  info.format.blue_mask_size};

    return {surface, format};
}

#endif  // __cplusplus

#endif  // LIBS_LIBC_SRC_INCLUDE_ALKOS_SYS_VIDEO_H_
