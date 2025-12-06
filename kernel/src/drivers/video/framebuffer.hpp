#ifndef KERNEL_SRC_DRIVERS_VIDEO_FRAMEBUFFER_HPP_
#define KERNEL_SRC_DRIVERS_VIDEO_FRAMEBUFFER_HPP_

#include <expected.hpp>
#include "graphics/color.hpp"
#include "graphics/surface.hpp"

namespace Drivers::Video
{

class Framebuffer
{
    public:
    Framebuffer() = default;

    void Init(Graphics::Surface s, Graphics::PixelFormat pf);

    // Accessors
    NODISCARD Graphics::Surface &GetSurface() { return surface_; }
    NODISCARD const Graphics::PixelFormat &GetFormat() const { return format_; }

    private:
    Graphics::Surface surface_{};
    Graphics::PixelFormat format_{};
};

}  // namespace Drivers::Video

#endif  // KERNEL_SRC_DRIVERS_VIDEO_FRAMEBUFFER_HPP_
