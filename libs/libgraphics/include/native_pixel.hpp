#ifndef LIBS_LIBGRAPHICS_INCLUDE_NATIVE_PIXEL_HPP_
#define LIBS_LIBGRAPHICS_INCLUDE_NATIVE_PIXEL_HPP_

#include <types.h>
#include <defines.hpp>

#include "color.hpp"

namespace Graphics
{

/**
 * @brief A wrapper around u32 that guarantees the value is formatted
 * correctly for the hardware framebuffer.
 */
struct PACK NativePixel {
    u32 value;

    NativePixel() = default;

    // Explicit constructor to prevent accidental integer assignment
    explicit constexpr NativePixel(u32 v) : value(v) {}

    bool operator==(const NativePixel &other) const = default;
    explicit constexpr operator u32() const { return value; }

    /**
     * @brief Converts a logical RGBA color to a hardware-specific NativePixel.
     */
    NODISCARD static constexpr NativePixel FromColor(const Color &c, const PixelFormat &pf)
    {
        const u32 r_mask = (1 << pf.red_mask_size) - 1;
        const u32 g_mask = (1 << pf.green_mask_size) - 1;
        const u32 b_mask = (1 << pf.blue_mask_size) - 1;

        u32 raw = (static_cast<u32>(c.r & r_mask) << pf.red_pos) |
                  (static_cast<u32>(c.g & g_mask) << pf.green_pos) |
                  (static_cast<u32>(c.b & b_mask) << pf.blue_pos);

        return NativePixel(raw);
    }
};

static_assert(sizeof(NativePixel) == sizeof(u32), "NativePixel must be 32-bit");

}  // namespace Graphics

#endif  // LIBS_LIBGRAPHICS_INCLUDE_NATIVE_PIXEL_HPP_
