#ifndef KERNEL_SRC_GRAPHICS_NATIVE_PIXEL_HPP_
#define KERNEL_SRC_GRAPHICS_NATIVE_PIXEL_HPP_

#include <defines.hpp>
#include <types.hpp>

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
};

static_assert(sizeof(NativePixel) == sizeof(u32), "NativePixel must be 32-bit");

}  // namespace Graphics

#endif  // KERNEL_SRC_GRAPHICS_NATIVE_PIXEL_HPP_
