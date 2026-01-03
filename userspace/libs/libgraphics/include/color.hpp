#ifndef KERNEL_SRC_GRAPHICS_COLOR_HPP_
#define KERNEL_SRC_GRAPHICS_COLOR_HPP_

#include <types.h>
#include <defines.hpp>

namespace Graphics
{

struct PACK Color {
    u8 r = 0;
    u8 g = 0;
    u8 b = 0;
    u8 a = 255;

    static constexpr Color Red() { return {255, 0, 0, 255}; }
    static constexpr Color Green() { return {0, 255, 0, 255}; }
    static constexpr Color Blue() { return {0, 0, 255, 255}; }
    static constexpr Color White() { return {255, 255, 255, 255}; }
    static constexpr Color Black() { return {0, 0, 0, 255}; }
};

struct PixelFormat {
    u8 red_pos;
    u8 red_mask_size;
    u8 green_pos;
    u8 green_mask_size;
    u8 blue_pos;
    u8 blue_mask_size;
    // We currently assume 32-bpp
};

}  // namespace Graphics

#endif  // KERNEL_SRC_GRAPHICS_COLOR_HPP_
