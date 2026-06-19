// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_ARCH_X86_64_BOOT_LIB_MEM_ERROR_HPP_
#define KERNEL_ARCH_X86_64_BOOT_LIB_MEM_ERROR_HPP_

enum class MemError {
    OutOfMemory,
    InvalidArgument,
};

#endif  // KERNEL_ARCH_X86_64_BOOT_LIB_MEM_ERROR_HPP_
