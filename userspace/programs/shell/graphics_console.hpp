// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef USERSPACE_PROGRAMS_SHELL_GRAPHICS_CONSOLE_HPP_
#define USERSPACE_PROGRAMS_SHELL_GRAPHICS_CONSOLE_HPP_

#include <color.hpp>
#include <font/psf2_font.hpp>
#include <span.hpp>

#include "painter.hpp"
#include "stream.hpp"

namespace System
{

// --------------------------------------------------------------------------------
// GraphicsConsole
// --------------------------------------------------------------------------------

class GraphicsConsole : public IO::IWriter
{
    public:
    GraphicsConsole(Graphics::Painter &painter, const Graphics::Psf2Font &font);

    // -------------------------------------------------------------------------
    // IWriter Interface
    // -------------------------------------------------------------------------

    IO::IoResult Write(std::span<const byte> buffer) override;

    // -------------------------------------------------------------------------
    // Console Operations
    // -------------------------------------------------------------------------

    void Clear();

    // -------------------------------------------------------------------------
    // Configuration
    // -------------------------------------------------------------------------

    void SetColors(Graphics::Color fg, Graphics::Color bg);

    private:
    // -------------------------------------------------------------------------
    // Internal Helpers
    // -------------------------------------------------------------------------

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
    u8 scale_{1};

    Graphics::Color fg_color_{Graphics::Color::White()};
    Graphics::Color bg_color_{Graphics::Color::Black()};
};

}  // namespace System

#endif  // USERSPACE_PROGRAMS_SHELL_GRAPHICS_CONSOLE_HPP_
