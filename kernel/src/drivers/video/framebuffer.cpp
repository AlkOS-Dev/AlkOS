#include "drivers/video/framebuffer.hpp"

using namespace Drivers::Video;

void Drivers::Video::Framebuffer::Init(Graphics::Surface s, Graphics::PixelFormat pf)
{
    surface_ = s;
    format_  = pf;
}
