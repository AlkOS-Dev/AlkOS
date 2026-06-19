// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_ARCH_X86_64_SRC_INCLUDE_MODELS_MEMORY_SPAN_HPP_
#define KERNEL_ARCH_X86_64_SRC_INCLUDE_MODELS_MEMORY_SPAN_HPP_

#include <types.h>
class MemorySpan
{
    public:
    //------------------------------------------------------------------------------//
    // Class Creation and Destruction
    //------------------------------------------------------------------------------//

    MemorySpan() : start{0}, end{0} {}
    MemorySpan(u64 start, u64 end) : start{start}, end{end} {}
    MemorySpan(const MemorySpan &)            = default;
    MemorySpan(MemorySpan &&)                 = default;
    MemorySpan &operator=(const MemorySpan &) = default;
    MemorySpan &operator=(MemorySpan &&)      = default;

    //------------------------------------------------------------------------------//
    // Public Methods
    //------------------------------------------------------------------------------//

    u64 GetSize() const { return end - start; }

    //------------------------------------------------------------------------------//
    // Public Fields
    //------------------------------------------------------------------------------//

    u64 start;
    u64 end;
};

#endif  // KERNEL_ARCH_X86_64_SRC_INCLUDE_MODELS_MEMORY_SPAN_HPP_
