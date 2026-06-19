// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_SRC_MODULES_HELPERS_HPP_
#define KERNEL_SRC_MODULES_HELPERS_HPP_

#define DEFINE_MODULE_FIELD(name_space, type) \
                                              \
    private:                                  \
    name_space::type type##_;                 \
                                              \
    public:                                   \
    NODISCARD FORCE_INLINE_F auto &Get##type() noexcept { return type##_; }

#endif  // KERNEL_SRC_MODULES_HELPERS_HPP_
