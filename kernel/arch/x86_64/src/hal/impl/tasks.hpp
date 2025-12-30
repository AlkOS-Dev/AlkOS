#ifndef KERNEL_ARCH_X86_64_SRC_HAL_IMPL_TASKS_HPP_
#define KERNEL_ARCH_X86_64_SRC_HAL_IMPL_TASKS_HPP_

#include <defines.hpp>
#include <types.hpp>

#include "hal/api/tasks.hpp"

namespace arch
{

struct Thread : ThreadAPI {
    alignas(64) byte fp_state[4096];

    u64 fs_base;
    u64 gs_base;

    FORCE_INLINE_F void InitMem()
    {
        fs_base = 0;
        gs_base = 0;
    }
};
struct Process {
};

}  // namespace arch

#endif  // KERNEL_ARCH_X86_64_SRC_HAL_IMPL_TASKS_HPP_
