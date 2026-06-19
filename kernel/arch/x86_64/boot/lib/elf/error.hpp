// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_ARCH_X86_64_BOOT_LIB_ELF_ERROR_HPP_
#define KERNEL_ARCH_X86_64_BOOT_LIB_ELF_ERROR_HPP_

namespace Elf64
{

enum class Error {
    InvalidElf,
    UnsupportedMachine,
    NullEntryPoint,
    DynamicHeaderNotFound,
    RelocationTableNotFound,
};

}  // namespace Elf64

#endif  // KERNEL_ARCH_X86_64_BOOT_LIB_ELF_ERROR_HPP_
