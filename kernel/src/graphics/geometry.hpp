#ifndef KERNEL_SRC_GRAPHICS_GEOMETRY_HPP_
#define KERNEL_SRC_GRAPHICS_GEOMETRY_HPP_

#include <string.hpp>
#include <types.hpp>

namespace Graphics
{

struct Point {
    i32 x;
    i32 y;
};

struct Rect {
    i32 x;
    i32 y;
    i32 w;
    i32 h;
};

struct TextCmd {
    i32 x;
    i32 y;
    std::string_view text;
    u8 scale = 1;
};

struct CharCmd {
    i32 x;
    i32 y;
    char c;
    u8 scale = 1;
};

struct Size {
    u32 width;
    u32 height;
};

}  // namespace Graphics

#endif  // KERNEL_SRC_GRAPHICS_GEOMETRY_HPP_
