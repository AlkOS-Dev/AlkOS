#ifndef KERNEL_SRC_GRAPHICS_PAINTER_TPP_
#define KERNEL_SRC_GRAPHICS_PAINTER_TPP_

#include "painter.hpp"

namespace Graphics
{

template <FontType FontT>
void Painter::DrawChar(i32 x, i32 y, char c, const FontT &font, u8 scale)
{
    Glyph glyph = font.GetGlyph(c);

    i32 scaled_width  = static_cast<i32>(glyph.width) * scale;
    i32 scaled_height = static_cast<i32>(glyph.height) * scale;

    // Skip if character is off-screen
    if (x >= static_cast<i32>(target_.GetWidth()) || y >= static_cast<i32>(target_.GetHeight()) ||
        x + scaled_width <= 0 || y + scaled_height <= 0) {
        return;
    }

    for (u32 row = 0; row < glyph.height; ++row) {
        const byte *row_data = glyph.buffer + (static_cast<size_t>(row * glyph.stride));

        for (u32 col = 0; col < glyph.width; ++col) {
            // Bit testing for PSF (Big Endian / MSB first)
            u32 byte_idx = col >> 3;
            u32 bit_idx  = 7 - (col & 7);

            bool is_set = (row_data[byte_idx] & (1 << bit_idx));

            if (is_set) {
                FillRect(
                    {static_cast<i32>(x + (col * scale)), static_cast<i32>(y + (row * scale)),
                     scale, scale}
                );
            }
        }
    }
}

template <FontType FontT>
void Painter::DrawString(const TextCmd &cmd, const FontT &font)
{
    if (cmd.text.empty()) {
        return;
    }

    i32 cursor_x          = cmd.x;
    i32 cursor_y          = cmd.y;
    const u32 line_height = font.GetHeight();

    for (char c : cmd.text) {
        if (c == '\n') {
            cursor_x = cmd.x;
            cursor_y += static_cast<i32>(line_height * cmd.scale);
        } else if (c == '\r') {
            cursor_x = cmd.x;
        } else {
            DrawChar(cursor_x, cursor_y, c, font, cmd.scale);
            cursor_x += static_cast<i32>(font.GetGlyph(c).advance * cmd.scale);
        }
    }
}

}  // namespace Graphics

#endif  // KERNEL_SRC_GRAPHICS_PAINTER_TPP_
