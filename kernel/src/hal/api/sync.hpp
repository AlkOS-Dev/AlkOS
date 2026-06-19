// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_SRC_HAL_API_SYNC_HPP_
#define KERNEL_SRC_HAL_API_SYNC_HPP_

#include <types.h>
#include <type_traits.hpp>

namespace hal
{
struct alignas(4) Atomic32 {
    using BaseT = i32;
    volatile BaseT value;
};

struct alignas(8) Atomic64 {
    using BaseT = i64;
    volatile BaseT value;
};

template <class T>
concept AtomicT = std::is_same_v<T, Atomic32> || std::is_same_v<T, Atomic64>;
}  // namespace hal

#endif  // KERNEL_SRC_HAL_API_SYNC_HPP_
