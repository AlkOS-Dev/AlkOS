#ifndef KERNEL_ARCH_X86_64_SRC_HAL_IMPL_TASKS_HPP_
#define KERNEL_ARCH_X86_64_SRC_HAL_IMPL_TASKS_HPP_

#include <types.hpp>
namespace arch
{

struct Thread {
    u64 fs_base;
    u64 gs_base;
};
struct Process {
};

}  // namespace arch

#endif  // KERNEL_ARCH_X86_64_SRC_HAL_IMPL_TASKS_HPP_
