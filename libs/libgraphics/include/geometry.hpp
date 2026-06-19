// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef LIBS_LIBGRAPHICS_INCLUDE_GEOMETRY_HPP_
#define LIBS_LIBGRAPHICS_INCLUDE_GEOMETRY_HPP_

#include <types.h>
#include <string.hpp>

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

#endif  // LIBS_LIBGRAPHICS_INCLUDE_GEOMETRY_HPP_
