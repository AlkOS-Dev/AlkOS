// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_ARCH_X86_64_BOOT_LOADER_32_SRC_ERROR_HPP_
#define KERNEL_ARCH_X86_64_BOOT_LOADER_32_SRC_ERROR_HPP_

namespace loader32
{

enum class Error {
    MultibootCheckFailed,
    CpuIdCheckFailed,
    LongModeCheckFailed,
    Loader64ModuleNotFound,
    ElfLoadFailed,
    MmapTagNotFound,
};

}  // namespace loader32

#endif  // KERNEL_ARCH_X86_64_BOOT_LOADER_32_SRC_ERROR_HPP_
