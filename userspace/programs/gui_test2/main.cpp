#include <platform.h>

extern "C" int main()
{
    __platform_debug_write("Starting GUI Test 2 ...\n");

    alkos::GraphicsContext ctx;
    if (!ctx.Init()) {
        __platform_debug_write("Failed to create graphics session!\n");
        return 1;
    }

    __platform_debug_write("Graphics session created successfully\n");

    const int square_size = 100;
    int square_x          = 0;
    int square_y          = 100;
    int dx                = 2;
    int dy                = 1;

    // Initial full screen clear
    ctx.Clear(alkos::Color::Black());
    ctx.BlitFull();  // Use full blit for initial clear

    int frame = 0;
    while (true) {
        // Clear old square position
        ctx.FillRect(square_x, square_y, square_size, square_size, alkos::Color::Black());

        // Update square position
        square_x += dx;
        square_y += dy;

        // Bounce off edges
        if (square_x + square_size >= static_cast<int>(ctx.Width()) || square_x < 0) {
            dx       = -dx;
            square_x = (square_x < 0) ? 0 : static_cast<int>(ctx.Width()) - square_size;
        }
        if (square_y + square_size >= static_cast<int>(ctx.Height()) || square_y < 0) {
            dy       = -dy;
            square_y = (square_y < 0) ? 0 : static_cast<int>(ctx.Height()) - square_size;
        }

        // Draw new square with gradient fill
        for (int y = 0; y < square_size; ++y) {
            for (int x = 0; x < square_size; ++x) {
                uint8_t r = (x + frame) & 0xFF;
                uint8_t g = (y + frame) & 0xFF;
                uint8_t b = (x + y) & 0xFF;
                ctx.SetPixel(square_x + x, square_y + y, alkos::Color(r, g, b));
            }
        }

        // Draw a border around the square
        ctx.DrawRect(square_x, square_y, square_size, square_size, alkos::Color::White());

        // Draw some decorative circles in the corners
        if (frame % 60 == 0) {
            int cx = square_x + square_size / 2;
            int cy = square_y + square_size / 2;
            ctx.DrawCircle(cx, cy, square_size / 4, alkos::Color::Yellow());
        }

        // Automatically blit only the dirty region (no manual calculation needed!)
        ctx.Blit();

        frame++;

        // Small delay to make animation visible
        for (volatile int i = 0; i < 100000; ++i);
    }

    return 0;
}
