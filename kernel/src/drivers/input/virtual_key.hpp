// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_SRC_DRIVERS_INPUT_VIRTUAL_KEY_HPP_
#define KERNEL_SRC_DRIVERS_INPUT_VIRTUAL_KEY_HPP_

#include <alkos/input.h>
#include <types.h>
#include <new.hpp>
#include <optional.hpp>

namespace Drivers::Input
{

/**
 * @brief Try to convert a virtual key to ASCII character
 * @param vk Virtual key code
 * @param modifiers Current keyboard modifier state
 * @return ASCII character if conversion is possible, std::nullopt otherwise
 */
std::optional<char> VirtualKeyToAscii(VirtualKey vk, KeyModifiers modifiers);

}  // namespace Drivers::Input

#endif  // KERNEL_SRC_DRIVERS_INPUT_VIRTUAL_KEY_HPP_
