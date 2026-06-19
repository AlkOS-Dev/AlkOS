// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef TESTS_LIBCPP_MOCKS_HAL_CONSTANTS_HPP_
#define TESTS_LIBCPP_MOCKS_HAL_CONSTANTS_HPP_

#include <cstddef>

namespace hal
{
static constexpr std::size_t kCacheLineSizeBytes = 64;
}

namespace arch
{
static constexpr std::size_t kCacheLineSizeBytes = 64;
}

#endif  // TESTS_LIBCPP_MOCKS_HAL_CONSTANTS_HPP_
