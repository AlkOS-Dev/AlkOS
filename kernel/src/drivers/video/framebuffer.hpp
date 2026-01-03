#ifndef KERNEL_SRC_DRIVERS_VIDEO_FRAMEBUFFER_HPP_
#define KERNEL_SRC_DRIVERS_VIDEO_FRAMEBUFFER_HPP_

#include <expected.hpp>
#include "graphics/color.hpp"
#include "graphics/native_pixel.hpp"
#include "graphics/surface.hpp"
#include "mem/heap.hpp"

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
