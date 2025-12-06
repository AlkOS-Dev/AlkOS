#ifndef KERNEL_SRC_MODULES_VIDEO_HPP_
#define KERNEL_SRC_MODULES_VIDEO_HPP_

#include <template_lib.hpp>

#include "boot_args.hpp"
#include "drivers/video/framebuffer.hpp"
#include "modules/helpers.hpp"

namespace internal
{

class VideoModule : template_lib::StaticSingletonHelper
{
    protected:
    explicit VideoModule(const BootArguments &args, Mem::Heap &hp) noexcept;

    DEFINE_MODULE_FIELD(Drivers::Video, Framebuffer);

    public:
    /// Returns the Surface for the physical screen
    Graphics::Surface &GetScreen() { return Framebuffer_.GetSurface(); }

    /// Returns the format required by the hardware (used to init Painters)
    const Graphics::PixelFormat &GetFormat() const { return Framebuffer_.GetFormat(); }

    void Flush() { Framebuffer_.Flush(); }
};

}  // namespace internal

using VideoModule = template_lib::StaticSingleton<internal::VideoModule>;

#endif  // KERNEL_SRC_MODULES_VIDEO_HPP_
