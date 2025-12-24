#ifndef KERNEL_SRC_CONSTANTS_HPP_
#define KERNEL_SRC_CONSTANTS_HPP_

#include "types.hpp"

static constexpr u16 kMaxProcesses    = 4096;
static constexpr u32 kMaxThreads      = kMaxProcesses * 2;
static constexpr u32 kStackSize       = 64 * 1024;  // TODO 8 meg
static constexpr u32 kKernelStackSize = 64 * 1024;  // TODO 1 meg
static constexpr u32 kStackAlignment  = 64;

#endif  // KERNEL_SRC_CONSTANTS_HPP_
