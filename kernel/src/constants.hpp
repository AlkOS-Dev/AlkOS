#ifndef KERNEL_SRC_CONSTANTS_HPP_
#define KERNEL_SRC_CONSTANTS_HPP_

#include <types.h>
#include <defines.hpp>

static constexpr u16 kMaxProcesses    = 4096;
static constexpr u32 kMaxThreads      = kMaxProcesses * 2;
static constexpr u32 kStackSize       = 64 * 1024;  // TODO 8 meg
static constexpr u32 kKernelStackSize = 64 * 1024;  // TODO 1 meg
static constexpr u32 kStackAlignment  = 64;

static constexpr u64 kUserSpaceStart          = 0x0;
static constexpr u64 kUserSpaceEndInclusive   = 0x00007FFFFFFFFFFF;
static constexpr u64 kUserSpaceEndExclusive   = kUserSpaceEndInclusive + 1;
static constexpr u64 kKernelSpaceStart        = 0xFFFF800000000000;
static constexpr u64 kKernelSpaceEndInclusive = 0xFFFFFFFFFFFFFFFF;

NODISCARD FAST_CALL bool IsKernelSpace(const u64 address) { return address >= kKernelSpaceStart; }

NODISCARD FAST_CALL bool IsKernelSpace(const void *address)
{
    return IsKernelSpace(reinterpret_cast<u64>(address));
}

#endif  // KERNEL_SRC_CONSTANTS_HPP_
