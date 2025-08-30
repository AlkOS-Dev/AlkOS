#ifndef ALKOS_KERNEL_ABI_MEMORY_MANAGER_HPP_
#define ALKOS_KERNEL_ABI_MEMORY_MANAGER_HPP_

#include <stddef.h>
#include <extensions/template_lib.hpp>

namespace memory
{

class MemoryManager : template_lib::StaticSingletonHelper
{
    protected:
    MemoryManager();
};

}  // namespace memory

using MemoryManager = template_lib::StaticSingleton<memory::MemoryManager>;

#endif  // ALKOS_KERNEL_ABI_MEMORY_MANAGER_HPP_
