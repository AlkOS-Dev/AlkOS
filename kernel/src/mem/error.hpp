// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_SRC_MEM_ERROR_HPP_
#define KERNEL_SRC_MEM_ERROR_HPP_

namespace Mem
{

enum class MemError {
    OutOfMemory,
    InvalidArgument,
    NotFound,
};

}  // namespace Mem

static constexpr const char *to_string(const Mem::MemError &error)
{
    switch (error) {
        case Mem::MemError::OutOfMemory:
            return "OutOfMemory";
        case Mem::MemError::NotFound:
            return "NotFound";
        case Mem::MemError::InvalidArgument:
            return "InvalidArgument";
    }

    return "unknown error";
}

#endif  // KERNEL_SRC_MEM_ERROR_HPP_
