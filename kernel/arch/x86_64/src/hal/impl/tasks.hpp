// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_ARCH_X86_64_SRC_HAL_IMPL_TASKS_HPP_
#define KERNEL_ARCH_X86_64_SRC_HAL_IMPL_TASKS_HPP_

#include <string.h>
#include <types.h>
#include <defines.hpp>

namespace arch
{

struct Thread {
    u64 fs_base;
    u64 gs_base;

    alignas(64) byte fp_state[4096];
};
struct Process {
};

}  // namespace arch

#endif  // KERNEL_ARCH_X86_64_SRC_HAL_IMPL_TASKS_HPP_
