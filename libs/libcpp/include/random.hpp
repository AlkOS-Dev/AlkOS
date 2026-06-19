// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef LIBS_LIBCPP_INCLUDE_RANDOM_HPP_
#define LIBS_LIBCPP_INCLUDE_RANDOM_HPP_

#include <types.h>
#include <defines.hpp>

class SimpleRandom
{
    public:
    explicit SimpleRandom(const u32 seed = 12345) : state(seed == 0 ? 12345 : seed) {}

    NODISCARD FORCE_INLINE_F u32 next()
    {
        state = state * 1664525 + 1013904223;
        return (state >> 16) % 32768;
    }

    private:
    u32 state;
};

#endif  // LIBS_LIBCPP_INCLUDE_RANDOM_HPP_
