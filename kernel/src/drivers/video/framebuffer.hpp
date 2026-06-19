// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_SRC_DRIVERS_VIDEO_FRAMEBUFFER_HPP_
#define KERNEL_SRC_DRIVERS_VIDEO_FRAMEBUFFER_HPP_

#include <color.hpp>
#include <surface.hpp>

#include <expected.hpp>

namespace Drivers::Video
{

struct FramebufferInfo {
    size_t width;
    size_t height;
    size_t pitch;
    Graphics::PixelFormat format;
};

class Framebuffer
{
    public:
    Framebuffer() = default;

    void Init(Graphics::Surface s, Graphics::PixelFormat pf);

    // Accessors
    NODISCARD Graphics::Surface &GetSurface() { return front_surface_; }
    NODISCARD const Graphics::PixelFormat &GetFormat() const { return format_; }

    NODISCARD FramebufferInfo GetInfo() const;
    NODISCARD size_t CalculateSize() const;

    private:
    Graphics::Surface front_surface_{};  // Physical VRAM
    Graphics::PixelFormat format_{};
};

}  // namespace Drivers::Video

#endif  // KERNEL_SRC_DRIVERS_VIDEO_FRAMEBUFFER_HPP_
