#include "modules/video.hpp"
#include "graphics/painter.hpp"
#include "mem/types.hpp"
#include "trace_framework.hpp"

using namespace Graphics;
using namespace Drivers;
using namespace Mem;

internal::VideoModule::VideoModule(const BootArguments &args, Heap &hp) noexcept
{
    DEBUG_INFO_GENERAL("VideoModule::VideoModule()");

    const auto &fb_args = args.fb_args;
    auto *fb_pptr       = static_cast<PPtr<u32>>(fb_args.base_address);

    R_ASSERT_NOT_NULL(fb_pptr, "VideoModule: Framebuffer is null");

    auto s  = Surface(Mem::PhysToVirt(fb_pptr), fb_args.width, fb_args.height, fb_args.pitch);
    auto pf = PixelFormat{
        .red_pos         = fb_args.red_pos,
        .red_mask_size   = fb_args.red_mask_size,
        .green_pos       = fb_args.green_pos,
        .green_mask_size = fb_args.green_mask_size,
        .blue_pos        = fb_args.blue_pos,
        .blue_mask_size  = fb_args.blue_mask_size,
    };

    Framebuffer_.Init(s, pf, hp);
    Graphics::Surface &screen = GetScreen();
    R_ASSERT_TRUE(screen.IsValid());

    Graphics::Painter p(screen, Framebuffer_.GetFormat());
    p.Clear(Graphics::Color::Black());
    Flush();
}
