#ifndef KERNEL_SRC_GRAPHICS_PAINTER_HPP_
#define KERNEL_SRC_GRAPHICS_PAINTER_HPP_

#include <concepts.hpp>
#include <span.hpp>
#include <string.hpp>
#include "graphics/color.hpp"
#include "graphics/font/glyph.hpp"
#include "graphics/geometry.hpp"
#include "graphics/native_pixel.hpp"
#include "graphics/surface.hpp"

namespace Graphics
{

/**
 * @brief Concept ensuring a type acts like a Font.
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

    void DrawPixel(Point p);
    void DrawRect(Rect r);
    void FillRect(Rect r);

    // -------------------------------------------------------------------------
    // Text Rendering
    // -------------------------------------------------------------------------

    /**
     * @brief Draws a single character using the provided font.
     * @tparam FontT A type satisfying the FontType concept (e.g., Psf2Font).
     */
    template <FontType FontT>
    void DrawChar(const CharCmd &cmd, const FontT &font);

    /**
     * @brief Draws a null-terminated string.
     * Handles newlines (\n) and carriage returns (\r).
     * @tparam FontT A type satisfying the FontType concept.
     */
    template <FontType FontT>
    void DrawString(const TextCmd &cmd, const FontT &font);

    private:
    void FillScanline(std::span<NativePixel> dest, NativePixel color);
    NODISCARD FORCE_INLINE_F NativePixel PackColor(Color c) const;

    Surface &target_;
    PixelFormat format_;
    NativePixel packed_color_;
};

}  // namespace Graphics

#include "graphics/painter.tpp"

#endif  // KERNEL_SRC_GRAPHICS_PAINTER_HPP_
