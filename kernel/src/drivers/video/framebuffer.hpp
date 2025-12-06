#ifndef KERNEL_SRC_DRIVERS_VIDEO_FRAMEBUFFER_HPP_
#define KERNEL_SRC_DRIVERS_VIDEO_FRAMEBUFFER_HPP_

#include <expected.hpp>
#include "graphics/color.hpp"
#include "graphics/surface.hpp"
#include "mem/heap.hpp"

namespace Drivers::Video
{

class Framebuffer
{
    public:
    Framebuffer() = default;

    void Init(Graphics::Surface s, Graphics::PixelFormat pf, Mem::Heap &hp);

    // Swap the backbuffer to the front (physical) buffer
    void Flush();

    // Accessors
    NODISCARD Graphics::Surface &GetSurface()
    {
        return back_surface_.IsValid() ? back_surface_ : front_surface_;
    }
    NODISCARD Graphics::Surface &GetFrontSurface() { return front_surface_; }
    NODISCARD const Graphics::PixelFormat &GetFormat() const { return format_; }

    private:
    Graphics::Surface front_surface_{};  // Physical VRAM
    Graphics::Surface back_surface_{};   // RAM Buffer
    Graphics::PixelFormat format_{};

    // We own the backbuffer memory
    Mem::VPtr<u32> backbuffer_mem_{nullptr};
};

}  // namespace Drivers::Video

#endif  // KERNEL_SRC_DRIVERS_VIDEO_FRAMEBUFFER_HPP_
