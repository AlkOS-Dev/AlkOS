#ifndef KERNEL_SRC_GRAPHICS_PAINTER_HPP_
#define KERNEL_SRC_GRAPHICS_PAINTER_HPP_

#include "graphics/color.hpp"
#include "graphics/surface.hpp"

namespace Graphics
{

class Painter
{
    public:
    Painter(Surface &target, const PixelFormat &format);

    void SetColor(Color color);

    // -------------------------------------------------------------------------
    // Primitives
    // -------------------------------------------------------------------------

    void Clear(Color color);

    // Safe drawing with clipping
    void DrawPixel(i32 x, i32 y);
    void DrawRect(i32 x, i32 y, i32 w, i32 h);
    void FillRect(i32 x, i32 y, i32 w, i32 h);

    private:
    NODISCARD FORCE_INLINE_F u32 PackColor(Color c) const;

    Surface &target_;
    PixelFormat format_;
    u32 packed_color_;
};

}  // namespace Graphics

#endif  // KERNEL_SRC_GRAPHICS_PAINTER_HPP_
