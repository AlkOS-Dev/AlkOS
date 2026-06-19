// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef LIBS_LIBCPP_INCLUDE_CSTDDEF_HPP_
#define LIBS_LIBCPP_INCLUDE_CSTDDEF_HPP_

#include <stddef.h>

namespace std
{
using nullptr_t   = ::nullptr_t;
using size_t      = ::size_t;
using ptrdiff_t   = ::ptrdiff_t;
using max_align_t = ::max_align_t;
}  // namespace std

#endif  // LIBS_LIBCPP_INCLUDE_CSTDDEF_HPP_
