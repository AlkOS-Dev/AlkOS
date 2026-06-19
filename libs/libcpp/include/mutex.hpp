// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef LIBS_LIBCPP_INCLUDE_MUTEX_HPP_
#define LIBS_LIBCPP_INCLUDE_MUTEX_HPP_

#include "concepts_ext.hpp"
#include "template_lib.hpp"

namespace std
{
// ------------------------------
// std::lock_guard
// ------------------------------

template <concepts_ext::LibCxxCompatibleMutex MutexT>
class lock_guard final : template_lib::NoCopy
{
    // ------------------------------
    // Constructors
    // ------------------------------

    public:
    explicit lock_guard(MutexT &m) : mutex_(m) { m.lock(); }

    // TODO: Add support for std::adopt_lock_t
    // lock_guard( MutexT& m, std::adopt_lock_t t );

    // ------------------------------
    // Destructor
    // ------------------------------

    ~lock_guard() { mutex_.unlock(); }

    // ------------------------------
    // Fields
    // ------------------------------

    private:
    MutexT &mutex_;
};
}  // namespace std

#endif  // LIBS_LIBCPP_INCLUDE_MUTEX_HPP_
