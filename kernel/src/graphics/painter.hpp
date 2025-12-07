#ifndef KERNEL_SRC_GRAPHICS_PAINTER_HPP_
#define KERNEL_SRC_GRAPHICS_PAINTER_HPP_

#include <concepts.hpp>
#include <string.hpp>
#include "graphics/color.hpp"
#include "graphics/font/glyph.hpp"
#include "graphics/surface.hpp"

namespace Graphics
{

/**
 * @brief Concept ensuring a type acts like a Font.
 * Must provide GetHeight() and GetGlyph(char).
 */
template <typename T>
concept FontType = requires(const T &t, char c) {
    { t.GetGlyph(c) } -> std::same_as<Glyph>;
    { t.GetHeight() } -> std::convertible_to<u32>;
};

class Painter
{
    public:
    Painter(Surface &target, const PixelFormat &format);

    void SetColor(Color color);

    // -------------------------------------------------------------------------
    // Primitives
    // -------------------------------------------------------------------------

    void Clear(Color color);

    void DrawPixel(i32 x, i32 y);
    void DrawRect(i32 x, i32 y, i32 w, i32 h);
    void FillRect(i32 x, i32 y, i32 w, i32 h);

    // -------------------------------------------------------------------------
    // Text Rendering (Templates)
    // -------------------------------------------------------------------------

    /**
     * @brief Draws a single character using the provided font.
     * @tparam FontT A type satisfying the FontType concept (e.g., Psf2Font).
     */
    template <FontType FontT>
    void DrawChar(i32 x, i32 y, char c, const FontT &font);

    /**
     * @brief Draws a null-terminated string.
     * Handles newlines (\n) and carriage returns (\r).
     * @tparam FontT A type satisfying the FontType concept.
     */
    template <FontType FontT>
    void DrawString(i32 x, i32 y, std::string_view str, const FontT &font);

    private:
    void FillScanline(u32 *dest, u32 count, u32 color);
    NODISCARD FORCE_INLINE_F u32 PackColor(Color c) const;

    Surface &target_;
    PixelFormat format_;
    u32 packed_color_;
};

}  // namespace Graphics

#include "graphics/painter.tpp"

#endif  // KERNEL_SRC_GRAPHICS_PAINTER_HPP_
