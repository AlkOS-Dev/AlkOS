// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef LIBS_LIBGRAPHICS_INCLUDE_FONT_GLYPH_HPP_
#define LIBS_LIBGRAPHICS_INCLUDE_FONT_GLYPH_HPP_

#include <types.h>

namespace Graphics
{

/**
 * @brief Represents a lightweight view into a specific character's bitmap data.
 * This struct does not own the memory.
 */
struct Glyph {
    const byte *buffer;  // Pointer to the raw bitmap bits
    u32 width;           // Width of the glyph in pixels
    u32 height;          // Height of the glyph in pixels
    u32 stride;          // Number of bytes per row in the buffer
    u32 advance;         // Horizontal pixels to advance cursor after drawing
};

}  // namespace Graphics

#endif  // LIBS_LIBGRAPHICS_INCLUDE_FONT_GLYPH_HPP_
