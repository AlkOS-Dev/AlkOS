#ifndef KERNEL_SRC_SYS_GRAPHICS_CONSOLE_HPP_
#define KERNEL_SRC_SYS_GRAPHICS_CONSOLE_HPP_

#include <graphics/color.hpp>
#include <graphics/font/psf2_font.hpp>
#include <graphics/painter.hpp>
#include <io/stream.hpp>

namespace System
{

class GraphicsConsole : public IO::IWriter
{
    public:
    GraphicsConsole(Graphics::Painter &painter, const Graphics::Psf2Font &font);

    // IWriter implementation for generic printf support
    IO::IoResult Write(std::span<const byte> buffer) override;

    // Direct character handling
    void Clear();

    // Configuration
    void SetColors(Graphics::Color fg, Graphics::Color bg);

    private:
    void InternalPutChar(char c);
    void NewLine();
    void ScrollUp();

    Graphics::Painter &painter_;
    const Graphics::Psf2Font &font_;

    // Cursor State
    u32 cursor_x_{0};
    u32 cursor_y_{0};

    // Cache dimensions
    u32 max_cols_{0};
    u32 max_rows_{0};
    u32 glyph_w_{0};
    u32 glyph_h_{0};

    Graphics::Color fg_color_{Graphics::Color::White()};
    Graphics::Color bg_color_{Graphics::Color::Black()};
};

}  // namespace System

#endif  // KERNEL_SRC_SYS_GRAPHICS_CONSOLE_HPP_
