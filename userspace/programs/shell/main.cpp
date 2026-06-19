// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#include <alkos/sys/thread.h>

#include "fonts/drdos8x8.hpp"
#include "shell.hpp"

#include <alkos/sys/video.h>

extern "C" int main()
{
    Graphics::Psf2Font font(drdos8x8_psfu);
    if (!font.IsValid()) {
        __platform_debug_write("Invalid font");
    }

    auto [screen, format] = GetVideoContext();
    Graphics::Painter painter(screen, format);
    System::GraphicsConsole console(painter, font);
    System::Shell shell(console);

    shell.Init();
    Blit();

    while (true) {
        shell.Update();
        Blit();

        NanoSleep(16'000'000);
    }

    return 0;
}
