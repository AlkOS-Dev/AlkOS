#include "graphics/painter.hpp"
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
    // TODO: masks should be used to clear bits before ORing.
    return (static_cast<u32>(c.r) << format_.red_pos) |
           (static_cast<u32>(c.g) << format_.green_pos) |
           (static_cast<u32>(c.b) << format_.blue_pos);
}

void Painter::DrawPixel(i32 x, i32 y)
{
    if (x < 0 || y < 0 || static_cast<u32>(x) >= target_.GetWidth() ||
        static_cast<u32>(y) >= target_.GetHeight()) {
        return;
    }
    target_.Pixel(static_cast<u32>(x), static_cast<u32>(y)) = packed_color_;
}

void Painter::Clear(Color color)
{
    u32 raw    = PackColor(color);
    u32 height = target_.GetHeight();
    u32 width  = target_.GetWidth();

    // Naive implementation. TODO: memset
    for (u32 y = 0; y < height; ++y) {
        u32 *line = target_.GetScanline(y);
        for (u32 x = 0; x < width; ++x) {
            line[x] = raw;
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

    // TODO: Memset?
    for (i32 row = 0; row < h; ++row) {
        u32 *line = target_.GetScanline(static_cast<u32>(y + row));
        for (i32 col = 0; col < w; ++col) {
            line[x + col] = packed_color_;
        }
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
