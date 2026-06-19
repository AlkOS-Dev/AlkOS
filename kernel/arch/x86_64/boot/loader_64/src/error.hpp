// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_ARCH_X86_64_BOOT_LOADER_64_SRC_ERROR_HPP_
#define KERNEL_ARCH_X86_64_BOOT_LOADER_64_SRC_ERROR_HPP_

namespace loader64
{

enum class Error {
    TransitionDataInvalid,
    KernelModuleNotFound,
    ElfBoundsInvalid,
    ElfLoadFailed,
};

}  // namespace loader64

#endif  // KERNEL_ARCH_X86_64_BOOT_LOADER_64_SRC_ERROR_HPP_
