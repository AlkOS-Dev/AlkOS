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
    // Wrappers removed as per request. Use GetFramebuffer() to access Framebuffer object.
};

}  // namespace internal

using VideoModule = template_lib::StaticSingleton<internal::VideoModule>;

#endif  // KERNEL_SRC_MODULES_VIDEO_HPP_
