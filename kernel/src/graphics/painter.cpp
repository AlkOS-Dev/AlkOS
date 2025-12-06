#include "graphics/painter.hpp"
#include <string.h>
#include <algorithm.hpp>

namespace Graphics
{

Painter::Painter(Surface &target, const PixelFormat &format) : target_(target), format_(format)
{
    ASSERT_TRUE(target_.IsValid(), "Painter initialized with invalid surface");
    SetColor(Color::White());
}

void Painter::SetColor(Color color) { packed_color_ = PackColor(color); }

u32 Painter::PackColor(Color c) const
{
    const u32 r_mask = (1 << format_.red_mask_size) - 1;
    const u32 g_mask = (1 << format_.green_mask_size) - 1;
    const u32 b_mask = (1 << format_.blue_mask_size) - 1;

    return (static_cast<u32>(c.r & r_mask) << format_.red_pos) |
           (static_cast<u32>(c.g & g_mask) << format_.green_pos) |
           (static_cast<u32>(c.b & b_mask) << format_.blue_pos);
}

void Painter::DrawPixel(i32 x, i32 y)
{
    if (x < 0 || y < 0 || static_cast<u32>(x) >= target_.GetWidth() ||
        static_cast<u32>(y) >= target_.GetHeight()) {
        return;
    }
    target_.Pixel(static_cast<u32>(x), static_cast<u32>(y)) = packed_color_;
}

void Painter::FillScanline(u32 *dest, u32 count, u32 color)
{
    // Check for byte-repeating pattern for memset optimization
    // e.g. 0x00000000, 0xFFFFFFFF, 0xABABABAB
    if ((color & 0xFF) == ((color >> 8) & 0xFF) && (color & 0xFF) == ((color >> 16) & 0xFF) &&
        (color & 0xFF) == ((color >> 24) & 0xFF)) {
        memset(dest, color & 0xFF, count * sizeof(u32));
    } else {
        // Compiler auto-vectorization usually handles this loop well
        for (u32 i = 0; i < count; ++i) {
            dest[i] = color;
        }
    }
}

void Painter::Clear(Color color)
{
    u32 raw    = PackColor(color);
    u32 height = target_.GetHeight();
    u32 width  = target_.GetWidth();
    u32 pitch  = target_.GetPitch();

    // Optimization: if buffer is contiguous (pitch == width * 4 for 32bpp)
    // we can clear it in one go.
    if (pitch == width * sizeof(u32)) {
        FillScanline(target_.GetScanline(0), width * height, raw);
    } else {
        for (u32 y = 0; y < height; ++y) {
            FillScanline(target_.GetScanline(y), width, raw);
        }
    }
}

void Painter::FillRect(i32 x, i32 y, i32 w, i32 h)
{
    // Clipping Logic
    if (x < 0) {
        w += x;
        x = 0;
    }
    if (y < 0) {
        h += y;
        y = 0;
    }

    if (static_cast<u32>(x) >= target_.GetWidth()) {
        return;
    }
    if (static_cast<u32>(y) >= target_.GetHeight()) {
        return;
    }

    // Clamp width/height to edges
    if (static_cast<u32>(x + w) > target_.GetWidth()) {
        w = target_.GetWidth() - x;
    }
    if (static_cast<u32>(y + h) > target_.GetHeight()) {
        h = target_.GetHeight() - y;
    }

    if (w <= 0 || h <= 0) {
        return;
    }

    for (i32 row = 0; row < h; ++row) {
        u32 *line = target_.GetScanline(static_cast<u32>(y + row));
        FillScanline(line + x, static_cast<u32>(w), packed_color_);
    }
}

void Painter::DrawRect(i32 x, i32 y, i32 w, i32 h)
{
    // Top
    FillRect(x, y, w, 1);
    // Bottom
    FillRect(x, y + h - 1, w, 1);
    // Left
    FillRect(x, y, 1, h);
    // Right
    FillRect(x + w - 1, y, 1, h);
}

}  // namespace Graphics
