// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_SRC_FS_COSTANTS_HPP_
#define KERNEL_SRC_FS_COSTANTS_HPP_

namespace Fs
{
inline constexpr size_t kMaxFdsPerProcess = 128;
inline constexpr size_t kMaxOpenFiles     = 1014;
inline constexpr size_t kMaxActiveFiles   = 512;
inline constexpr size_t kStdioBufferSize  = 4096;
}  // namespace Fs

#endif  // KERNEL_SRC_FS_COSTANTS_HPP_
