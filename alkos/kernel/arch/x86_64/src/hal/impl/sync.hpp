#ifndef ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_SYNC_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_SYNC_HPP_

#include <extensions/defines.hpp>

namespace arch
{
FAST_CALL void FullMemFence() { __asm__ volatile("mfence" ::: "memory"); }

FAST_CALL void LoadMemFence() { __asm__ volatile("lfence" ::: "memory"); }

FAST_CALL void SaveMemFence() { __asm__ volatile("" ::: "memory"); }
}  // namespace arch

#endif  // ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_SYNC_HPP_
