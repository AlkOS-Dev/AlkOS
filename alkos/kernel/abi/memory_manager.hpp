#ifndef ALKOS_ALKOS_KERNEL_ABI_MEMORY_MANAGER_HPP_
#define ALKOS_ALKOS_KERNEL_ABI_MEMORY_MANAGER_HPP_

#include <stddef.h>
#include <extensions/template_lib.hpp>

namespace memory
{

class MemoryManager : TemplateLib::StaticSingletonHelper
{
    protected:
    MemoryManager();
};

}  // namespace memory

using MemoryManager = TemplateLib::StaticSingleton<memory::MemoryManager>;

#endif  // ALKOS_ALKOS_KERNEL_ABI_MEMORY_MANAGER_HPP_
