#include "painter.hpp"

#include <string.h>
#include <algorithm.hpp>

namespace Graphics
{

Painter::Painter(Surface &target, const PixelFormat &format) : target_(target), format_(format)
{
    ASSERT_TRUE(target_.IsValid(), "Painter initialized with invalid surface");
    SetColor(Color::White());
}

void Painter::SetColor(Color color) { packed_color_ = NativePixel::FromColor(color, format_); }

void Painter::DrawPixel(Point p)
{
    i32 x = p.x;
    i32 y = p.y;
    if (x < 0 || y < 0 || static_cast<u32>(x) >= target_.GetWidth() ||
        static_cast<u32>(y) >= target_.GetHeight()) {
        return;
    }
    target_.Pixel(static_cast<u32>(x), static_cast<u32>(y)) = packed_color_;
}

void Painter::FillScanline(std::span<NativePixel> dest, NativePixel color)
{
    u32 raw = color.value;

    // Optimization: If byte pattern is repeating, use memset
    if ((raw & 0xFF) == ((raw >> 8) & 0xFF) && (raw & 0xFF) == ((raw >> 16) & 0xFF) &&
        (raw & 0xFF) == ((raw >> 24) & 0xFF)) {
        memset(dest.data(), raw & 0xFF, dest.size_bytes());
    } else {
        for (auto &px : dest) {
            px = color;
        }
    }
}

void Painter::Clear(Color color)
{
    NativePixel raw = NativePixel::FromColor(color, format_);
    u32 height      = target_.GetHeight();
    u32 width       = target_.GetWidth();
    u32 pitch       = target_.GetPitch();

    if (pitch == width * sizeof(NativePixel)) {
        std::span<NativePixel> first_line = target_.GetScanline(0);
        std::span<NativePixel> full_buffer(first_line.data(), width * height);
        FillScanline(full_buffer, raw);
    } else {
        for (u32 y = 0; y < height; ++y) {
            FillScanline(target_.GetScanline(y), raw);
        }
    }
}

void Painter::FillRect(Rect r)
{
    i32 x = r.x;
    i32 y = r.y;
    i32 w = r.w;
    i32 h = r.h;

    if (x < 0) {
        w += x;
        x = 0;
    }
    if (y < 0) {
        h += y;
        y = 0;
    }

    if (static_cast<u32>(x) >= target_.GetWidth() || static_cast<u32>(y) >= target_.GetHeight()) {
        return;
    }

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
        std::span<NativePixel> line = target_.GetScanline(static_cast<u32>(y + row));
        FillScanline(line.subspan(static_cast<size_t>(x), static_cast<size_t>(w)), packed_color_);
    }
}

void Painter::DrawRect(Rect r)
{
    i32 x = r.x;
    i32 y = r.y;
    i32 w = r.w;
    i32 h = r.h;

    FillRect({x, y, w, 1});
    FillRect({x, y + h - 1, w, 1});
    FillRect({x, y, 1, h});
    FillRect({x + w - 1, y, 1, h});
}

}  // namespace Graphics
