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

        // TODO: Replace with CpuHalt or smth like scheduler sleep.
        for (volatile i32 i = 0; i < 10000; ++i) {
        }
    }

    return 0;
}
