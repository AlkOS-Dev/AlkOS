// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef LIBS_LIBTEMPLATE_INCLUDE_TEMPLATE_INTEGRAL_STORAGE_HPP_
#define LIBS_LIBTEMPLATE_INCLUDE_TEMPLATE_INTEGRAL_STORAGE_HPP_

#include "bits_ext.hpp"

namespace template_lib
{
template <size_t kNum>
using MinimalUnsignedStorage_t = UnsignedIntegral_t<(std::bit_width(kNum) + 7) / 8>;

}  // namespace template_lib

#endif  // LIBS_LIBTEMPLATE_INCLUDE_TEMPLATE_INTEGRAL_STORAGE_HPP_
