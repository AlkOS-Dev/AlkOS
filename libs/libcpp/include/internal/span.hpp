// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef LIBS_LIBCPP_INCLUDE_INTERNAL_SPAN_HPP_
#define LIBS_LIBCPP_INCLUDE_INTERNAL_SPAN_HPP_

#include <assert.h>
#include <span.hpp>

namespace internal
{

template <class T, class U, std::size_t N>
    requires std::is_trivial_v<T> && std::is_standard_layout_v<T> && (!std::is_const_v<U>)
auto &get(std::span<U, N> s, size_t offset = 0)
{
    ASSERT_LE(offset + sizeof(T), s.size_bytes(), "Offset out of bounds");
    return *reinterpret_cast<T *>(std::as_writable_bytes(s).data() + offset);
}

template <class T, class U, std::size_t N>
    requires std::is_trivial_v<T> && std::is_standard_layout_v<T> && std::is_const_v<U>
const T &get(std::span<U, N> s, size_t offset = 0)
{
    ASSERT_LE(offset + sizeof(T), s.size_bytes(), "Offset out of bounds");
    return *reinterpret_cast<const T *>(std::as_bytes(s).data() + offset);
}

}  // namespace internal

#endif  // LIBS_LIBCPP_INCLUDE_INTERNAL_SPAN_HPP_
