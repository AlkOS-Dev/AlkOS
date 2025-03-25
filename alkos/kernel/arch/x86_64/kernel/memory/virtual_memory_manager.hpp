#ifndef ALKOS_ALKOS_KERNEL_ARCH_X86_64_KERNEL_MEMORY_MANAGEMENT_VIRTUAL_MEMORY_MANAGER_HPP_
#define ALKOS_ALKOS_KERNEL_ARCH_X86_64_KERNEL_MEMORY_MANAGEMENT_VIRTUAL_MEMORY_MANAGER_HPP_

#include <extensions/template_lib.hpp>
#include <memory/page_tables_layout.hpp>

namespace memory
{

class VirtualMemoryManager : TemplateLib::StaticSingletonHelper
{
    public:
    //------------------------------------------------------------------------------//
    // Internal Types
    //------------------------------------------------------------------------------//
    //------------------------------------------------------------------------------//
    // Class Creation and Destruction
    //------------------------------------------------------------------------------//
    //------------------------------------------------------------------------------//
    // Public Methods
    //------------------------------------------------------------------------------//

    void Allocate(u64 virtual_address, u64 flags);
    void Free(u64 virtual_address);

    PML4_t& GetPml4Table();

    //------------------------------------------------------------------------------//
    // Public Fields
    //------------------------------------------------------------------------------//

    private:
    //------------------------------------------------------------------------------//
    // Private Methods
    //------------------------------------------------------------------------------//
    //------------------------------------------------------------------------------//
    // Private Fields
    //------------------------------------------------------------------------------//

    PML4_t pml4_{};

    //------------------------------------------------------------------------------//
    // Helpers
    //------------------------------------------------------------------------------//
};

}  // namespace memory

#include "virtual_memory_manager.tpp"

using VirtualMemoryManager = TemplateLib::StaticSingleton<memory::VirtualMemoryManager>;

#endif  // ALKOS_ALKOS_KERNEL_ARCH_X86_64_KERNEL_MEMORY_MANAGEMENT_VIRTUAL_MEMORY_MANAGER_HPP_
