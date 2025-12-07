#ifndef KERNEL_SRC_GRAPHICS_PAINTER_TPP_
#define KERNEL_SRC_GRAPHICS_PAINTER_TPP_

#include "painter.hpp"

namespace Graphics
{

template <FontType FontT>
void Painter::DrawChar(i32 x, i32 y, char c, const FontT &font)
{
    Glyph glyph = font.GetGlyph(c);

    // Skip if the entire character is off-screen
    if (x >= static_cast<i32>(target_.GetWidth()) || y >= static_cast<i32>(target_.GetHeight()) ||
        x + static_cast<i32>(glyph.width) <= 0 || y + static_cast<i32>(glyph.height) <= 0) {
        return;
    }

    for (u32 row = 0; row < glyph.height; ++row) {
        i32 screen_y = y + static_cast<i32>(row);

        // Vertical Clipping
        if (screen_y < 0) {
            continue;
        }
        if (screen_y >= static_cast<i32>(target_.GetHeight())) {
            break;
        }

        const byte *row_data = glyph.buffer + (static_cast<size_t>(row * glyph.stride));

        for (u32 col = 0; col < glyph.width; ++col) {
            i32 screen_x = x + static_cast<i32>(col);

            // Horizontal Clipping
            if (screen_x < 0) {
                continue;
            }
            if (screen_x >= static_cast<i32>(target_.GetWidth())) {
                break;
            }

            // Bit testing for PSF (Big Endian / MSB first)
            // Byte index = col / 8
            // Bit index  = 7 - (col % 8)
            u32 byte_idx = col >> 3;
            u32 bit_idx  = 7 - (col & 7);

            bool is_set = (row_data[byte_idx] & (1 << bit_idx));

            if (is_set) {
                target_.Pixel(static_cast<u32>(screen_x), static_cast<u32>(screen_y)) =
                    packed_color_;
            }
        }
    }
}

template <FontType FontT>
void Painter::DrawString(i32 x, i32 y, const char *str, const FontT &font)
{
    if (!str) {
        return;
    }

    i32 cursor_x          = x;
    i32 cursor_y          = y;
    const u32 line_height = font.GetHeight();

    while (*str) {
        char c = *str++;

        if (c == '\n') {
            cursor_x = x;
            cursor_y += static_cast<i32>(line_height);
        } else if (c == '\r') {
            cursor_x = x;
        } else {
            DrawChar(cursor_x, cursor_y, c, font);
            cursor_x += static_cast<i32>(font.GetGlyph(c).advance);
        }
    }
}

}  // namespace Graphics

#endif  // KERNEL_SRC_GRAPHICS_PAINTER_TPP_
