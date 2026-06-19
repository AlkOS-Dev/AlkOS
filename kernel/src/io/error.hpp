// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_SRC_IO_ERROR_HPP_
#define KERNEL_SRC_IO_ERROR_HPP_

namespace IO
{

enum class Error {
    None,
    Retry,        // Resource busy/buffer full (Non-blocking)
    EndOfFile,    // Connection closed
    DeviceError,  // Hardware fault
    InvalidInput
};

}

#endif  // KERNEL_SRC_IO_ERROR_HPP_
