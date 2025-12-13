#ifndef KERNEL_SRC_GRAPHICS_FONT_PSF2_FONT_HPP_
#define KERNEL_SRC_GRAPHICS_FONT_PSF2_FONT_HPP_

#include <assert.h>
#include <defines.hpp>
#include <span.hpp>
#include <string.hpp>
#include <types.hpp>

#include "graphics/font/glyph.hpp"
#include "graphics/geometry.hpp"

namespace Graphics
{

/**
 * @brief PC Screen Font v2 Header Layout
 */
struct PACK Psf2Header {
    u32 magic;        // Magic bytes: 0x864ab572 (Little Endian)
    u32 version;      // Zero
    u32 header_size;  // Offset to glyphs
    u32 flags;        // Font flags
    u32 length;       // Number of glyphs
    u32 char_size;    // Number of bytes per glyph
    u32 height;       // Height in pixels
    u32 width;        // Width in pixels
};

/**
 * @brief Zero-overhead parser for PSF2 font data.
 * @note This class does not own the data buffer.
 */
class Psf2Font
{
    public:
    static constexpr u32 kPsf2Magic = 0x864ab572;

    constexpr Psf2Font(std::span<const u8> data) : data_(data)
    {
        // TODO: In a constexpr context, static assert
        // TODO: Runtime checks can be added if instantiated dynamically.
    }

    [[nodiscard]] FORCE_INLINE_F const Psf2Header &Header() const
    {
        return *reinterpret_cast<const Psf2Header *>(data_.data());
    }

    [[nodiscard]] bool IsValid() const
    {
        if (data_.size() < sizeof(Psf2Header)) {
            return false;
        }
        return Header().magic == kPsf2Magic;
    }

    [[nodiscard]] FORCE_INLINE_F u32 GetWidth() const { return Header().width; }
    [[nodiscard]] FORCE_INLINE_F u32 GetHeight() const { return Header().height; }

    /**
     * @brief Retrieves the bitmap view for a specific character.
     * @param c The character code (ASCII/Extended ASCII).
     * @return Glyph structure containing geometry and data pointer.
     */
    [[nodiscard]] Glyph GetGlyph(char c) const
    {
        const auto &hdr = Header();
        u32 index       = static_cast<u8>(c);

        // Fallback for out of range
        if (index >= hdr.length) {
            index = 0;
        }

        const u8 *glyph_start = data_.data() + hdr.header_size + (index * hdr.char_size);

        // PSF2 rows are byte-aligned.
        // Stride = ceil(width / 8)
        u32 stride = (hdr.width + 7) >> 3;

        return Glyph{
            .buffer  = glyph_start,
            .width   = hdr.width,
            .height  = hdr.height,
            .stride  = stride,
            .advance = hdr.width
        };
    }

    /**
     * @brief Calculates the bounding box size of the text if it were drawn.
     * @param text The string to measure.
     * @param scale The scaling factor used for drawing.
     * @return The width and height in pixels.
     */
    NODISCARD Size MeasureString(std::string_view text, u8 scale = 1) const
    {
        if (text.empty()) {
            return {0, 0};
        }

        const auto &hdr = Header();
        u32 cursor_x    = 0;
        u32 cursor_y    = 0;
        u32 max_width   = 0;

        u32 total_height = hdr.height * scale;

        for (char c : text) {
            if (c == '\n') {
                max_width = std::max(max_width, cursor_x);
                cursor_x  = 0;
                cursor_y += hdr.height * scale;
                total_height = std::max(total_height, cursor_y + (hdr.height * scale));
            } else if (c == '\r') {
                cursor_x = 0;
            } else {
                cursor_x += hdr.width * scale;
            }
        }

        max_width = std::max(max_width, cursor_x);

        return {max_width, total_height};
    }

    private:
    std::span<const u8> data_;
};

}  // namespace Graphics

#endif  // KERNEL_SRC_GRAPHICS_FONT_PSF2_FONT_HPP_
