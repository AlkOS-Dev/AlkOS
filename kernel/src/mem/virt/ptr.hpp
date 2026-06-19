// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_SRC_MEM_VIRT_PTR_HPP_
#define KERNEL_SRC_MEM_VIRT_PTR_HPP_

#include <types.h>
#include <type_traits.hpp>
#include "hal/constants.hpp"

namespace mem
{

template <typename T>
using VirtualPtr = T *;

}  // namespace mem

#endif  // KERNEL_SRC_MEM_VIRT_PTR_HPP_
