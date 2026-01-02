#include "alkos/graphics_console.hpp"

#include <string.h>
#include <algorithm.hpp>

namespace System
{

GraphicsConsole::GraphicsConsole(Graphics::Painter &painter, const Graphics::Psf2Font &font)
    : painter_(painter), font_(font)
{
    // Auto-scale based on resolution
    scale_ = static_cast<u8>(std::max(1u, painter_.GetTarget().GetWidth() / 400));

    glyph_w_ = font_.GetWidth() * scale_;
    glyph_h_ = font_.GetHeight() * scale_;

    // Calculate grid size
    max_cols_ = painter_.GetTarget().GetWidth() / glyph_w_;
    max_rows_ = painter_.GetTarget().GetHeight() / glyph_h_;

    Clear();
}

void GraphicsConsole::SetColors(Graphics::Color fg, Graphics::Color bg)
{
    fg_color_ = fg;
    bg_color_ = bg;
}

void GraphicsConsole::Clear()
{
    painter_.Clear(bg_color_);
    cursor_x_ = 0;
    cursor_y_ = 0;
}

void GraphicsConsole::ScrollUp()
{
    // Scroll by one line height
    auto &surface      = painter_.GetTarget();
    u32 pitch          = surface.GetPitch();
    u32 bytes_per_line = pitch * glyph_h_;
    u32 total_height   = surface.GetHeight();

    u8 *raw_buf = reinterpret_cast<u8 *>(surface.GetRawBuffer());

    // Move memory up
    // Dest: Start of buffer
    // Src: Start of buffer + one line of pixels
    // Size: Total size - one line of pixels
    memmove(
        raw_buf, raw_buf + bytes_per_line, static_cast<size_t>(pitch * (total_height - glyph_h_))
    );

    // Clear bottom area using the painter to ensure correct color format
    painter_.SetColor(bg_color_);
    painter_.FillRect(
        {0, static_cast<i32>(total_height - glyph_h_), static_cast<i32>(surface.GetWidth()),
         static_cast<i32>(glyph_h_)}
    );
}

void GraphicsConsole::NewLine()
{
    cursor_x_ = 0;
    cursor_y_++;

    if (cursor_y_ >= max_rows_) {
        cursor_y_ = max_rows_ - 1;
        ScrollUp();
    }
}

void GraphicsConsole::InternalPutChar(char c)
{
    if (c == '\n') {
        NewLine();
        return;
    }

    if (c == '\r') {
        cursor_x_ = 0;
        return;
    }

    if (c == '\b') {
        if (cursor_x_ > 0) {
            cursor_x_--;
            // Erase char
            painter_.SetColor(bg_color_);
            painter_.FillRect(
                {static_cast<i32>(cursor_x_ * glyph_w_), static_cast<i32>(cursor_y_ * glyph_h_),
                 static_cast<i32>(glyph_w_), static_cast<i32>(glyph_h_)}
            );
        }
        return;
    }

    // Draw the character
    Graphics::CharCmd cmd{
        .x     = static_cast<i32>(cursor_x_ * glyph_w_),
        .y     = static_cast<i32>(cursor_y_ * glyph_h_),
        .c     = c,
        .scale = scale_
    };

    // Clear background for char
    painter_.SetColor(bg_color_);
    painter_.FillRect({cmd.x, cmd.y, static_cast<i32>(glyph_w_), static_cast<i32>(glyph_h_)});

    // Draw foreground
    painter_.SetColor(fg_color_);
    painter_.DrawChar(cmd, font_);

    cursor_x_++;
    if (cursor_x_ >= max_cols_) {
        NewLine();
    }
}

IO::IoResult GraphicsConsole::Write(std::span<const byte> buffer)
{
    for (const auto &b : buffer) {
        InternalPutChar(static_cast<char>(b));
    }
    return buffer.size();
}

}  // namespace System
