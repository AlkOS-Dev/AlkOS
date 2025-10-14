#ifndef ALKOS_KERNEL_INCLUDE_MODULES_MEMORY_HPP_
#define ALKOS_KERNEL_INCLUDE_MODULES_MEMORY_HPP_

#include <extensions/template_lib.hpp>

#include "hal/kernel.hpp"
#include "mem/vmm/vmm.hpp"
#include "modules/helpers.hpp"

namespace internal
{
class MemoryModule : template_lib::StaticSingletonHelper
{
    // -------------------------------------
    // Protected singleton constructor
    // -------------------------------------

    protected:
    explicit MemoryModule(const hal::KernelArguments& args) noexcept;

    // ------------------------------
    // Module fields
    // ------------------------------

    DEFINE_MODULE_FIELD(mem, VirtualMemoryManager)
};
}  // namespace internal

using MemoryModule = template_lib::StaticSingleton<internal::MemoryModule>;

#endif  // ALKOS_KERNEL_INCLUDE_MODULES_MEMORY_HPP_
