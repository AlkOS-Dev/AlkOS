#ifndef KERNEL_SRC_GRAPHICS_PAINTER_HPP_
#define KERNEL_SRC_GRAPHICS_PAINTER_HPP_

#include <concepts.hpp>
#include <span.hpp>

#include "color.hpp"
#include "font/glyph.hpp"
#include "geometry.hpp"
#include "native_pixel.hpp"
#include "surface.hpp"

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

    template <FontType FontT>
    void DrawChar(const CharCmd &cmd, const FontT &font);

    template <FontType FontT>
    void DrawString(const TextCmd &cmd, const FontT &font);

    // -------------------------------------------------------------------------
    // Accessors
    // -------------------------------------------------------------------------

    NODISCARD Surface &GetTarget() { return target_; }
    NODISCARD const PixelFormat &GetFormat() const { return format_; }

    private:
    void FillScanline(std::span<NativePixel> dest, NativePixel color);

    Surface &target_;
    PixelFormat format_;
    NativePixel packed_color_;
};

}  // namespace Graphics

#include "painter.tpp"

#endif  // KERNEL_SRC_GRAPHICS_PAINTER_HPP_
