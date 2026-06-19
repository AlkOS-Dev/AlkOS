// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_SRC_HAL_API_CORE_HPP_
#define KERNEL_SRC_HAL_API_CORE_HPP_

#include <defines.hpp>

namespace arch
{

class Core;

struct CoreAPI {
    /* Should perform full initialisation of single core */
    void EnableCore();
};

struct CoreControllerAPI {
};

NODISCARD WRAP_CALL u32 GetCurrentCoreId();

}  // namespace arch

#endif  // KERNEL_SRC_HAL_API_CORE_HPP_
