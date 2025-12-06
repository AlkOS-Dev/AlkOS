#include "drivers/video/framebuffer.hpp"
#include "defines.hpp"
#include "mem/heap.hpp"
#include "trace_framework.hpp"

using namespace Drivers::Video;

void Framebuffer::Init(Graphics::Surface s, Graphics::PixelFormat pf, Mem::Heap &hp)
{
    front_surface_ = s;
    format_        = pf;

    // Backbuffer allocation
    size_t width       = front_surface_.GetWidth();
    size_t height      = front_surface_.GetHeight();
    size_t buffer_size = width * height * sizeof(u32);

    auto alloc_res = hp.Malloc(buffer_size);
    if (!alloc_res) {
        TRACE_WARN_VIDEO("Backbuffer allocation failed, running in single-buffer mode");
        return;
    }

    backbuffer_mem_ = static_cast<Mem::VPtr<u32>>(*alloc_res);
    back_surface_   = Graphics::Surface(backbuffer_mem_, width, height, width * sizeof(u32));
}

void Framebuffer::Flush()
{
    if (back_surface_.IsValid()) {
        front_surface_.CopyFrom(back_surface_);
    }
}
