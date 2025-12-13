#ifndef KERNEL_SRC_GRAPHICS_FONT_GLYPH_HPP_
#define KERNEL_SRC_GRAPHICS_FONT_GLYPH_HPP_

#include <types.hpp>

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

#endif  // KERNEL_SRC_GRAPHICS_FONT_GLYPH_HPP_
