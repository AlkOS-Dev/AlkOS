#include <platform.h>
#include <stdio.h>
#include <sys/video.h>
#include <types.hpp>

extern "C" int main()
{
    __platform_debug_write("Starting GUI Test...\n");

    GuiBufferInfo info = {};
    __platform_create_graphic_session(&info);

    if (info.buffer_ptr == nullptr) {
        __platform_debug_write("Failed to create graphics session!\n");
        return 1;
    }

    // cast void* to something writable (assuming 32bpp / 4 bytes)
    u32 *fb = static_cast<u32 *>(info.buffer_ptr);

    int color_offset = 0;

    // Use pixel format info
    const auto &fmt = info.format;

    while (true) {
        // Simple rendering pattern
        for (size_t y = 0; y < info.height; ++y) {
            for (size_t x = 0; x < info.width; ++x) {
                // Calculate pixel index.
                // Note: Pitch is in bytes, so we divide by 4 for u32 pointer arithmetic
                size_t index = (y * (info.pitch / 4)) + x;

                u8 r = (x + color_offset) & 0xFF;
                u8 g = (y + color_offset) & 0xFF;
                u8 b = (x + y) & 0xFF;

                // Construct pixel using format info
                u32 pixel = 0;
                pixel |= (static_cast<u32>(r) & ((1 << fmt.red_mask_size) - 1)) << fmt.red_pos;
                pixel |= (static_cast<u32>(g) & ((1 << fmt.green_mask_size) - 1)) << fmt.green_pos;
                pixel |= (static_cast<u32>(b) & ((1 << fmt.blue_mask_size) - 1)) << fmt.blue_pos;

                fb[index] = pixel;
            }
        }

        // Blit to screen
        __platform_blit();

        color_offset++;
    }

    return 0;
}
