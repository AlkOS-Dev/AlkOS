// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#include "drivers/video/framebuffer.hpp"

#include "native_pixel.hpp"

using namespace Drivers::Video;

void Framebuffer::Init(Graphics::Surface s, Graphics::PixelFormat pf)
{
    front_surface_ = s;
    format_        = pf;
}

FramebufferInfo Framebuffer::GetInfo() const
{
    return FramebufferInfo{
        .width  = front_surface_.GetWidth(),
        .height = front_surface_.GetHeight(),
        .pitch  = front_surface_.GetPitch(),
        .format = format_
    };
}

size_t Framebuffer::CalculateSize() const
{
    // Return size matching the framebuffer layout (including pitch)
    return static_cast<size_t>(front_surface_.GetPitch()) * front_surface_.GetHeight();
}
