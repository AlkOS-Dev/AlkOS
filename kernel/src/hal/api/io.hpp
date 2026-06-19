// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_SRC_HAL_API_IO_HPP_
#define KERNEL_SRC_HAL_API_IO_HPP_

#include <types.h>
#include <concepts.hpp>

namespace arch
{

template <typename T>
concept IoT = std::is_same_v<T, u8> || std::is_same_v<T, u16> || std::is_same_v<T, u32>;

template <IoT T>
void IoWrite(size_t addr, T value);
template <IoT T>
T IoRead(size_t addr);

}  // namespace arch

namespace hal
{
using arch::IoT;
}

#endif  // KERNEL_SRC_HAL_API_IO_HPP_
