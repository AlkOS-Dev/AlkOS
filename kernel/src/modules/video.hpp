// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_SRC_MODULES_VIDEO_HPP_
#define KERNEL_SRC_MODULES_VIDEO_HPP_

#include <template_lib.hpp>

#include "boot_args.hpp"
#include "drivers/video/framebuffer.hpp"
#include "mem/heap.hpp"
#include "modules/helpers.hpp"
#include "video/window_manager.hpp"

namespace internal
{

class VideoModule : template_lib::StaticSingletonHelper
{
    protected:
    explicit VideoModule(const BootArguments &args, Mem::Heap &hp) noexcept;

    public:
    DEFINE_MODULE_FIELD(Drivers::Video, Framebuffer);
    DEFINE_MODULE_FIELD(Video, WindowManager);
};

}  // namespace internal

using VideoModule = template_lib::StaticSingleton<internal::VideoModule>;

#endif  // KERNEL_SRC_MODULES_VIDEO_HPP_
