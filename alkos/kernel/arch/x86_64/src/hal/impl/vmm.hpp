#ifndef ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEM_VMM_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEM_VMM_HPP_

#include <hal/api/vmm.hpp>
#include <extensions/utility.hpp>

#include "mem/settings.hpp"

namespace arch
{

class VirtualMemoryManager : public internal::VmmImpl
{
    public:
    template <typename... Args>
    VirtualMemoryManager(Args &&...args) : internal::VmmImpl(std::forward<Args>(args)...)
    {
    }
};

using VmmConfig = internal::VmmConfig;

}  // namespace arch

#endif  // ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEM_VMM_HPP_
