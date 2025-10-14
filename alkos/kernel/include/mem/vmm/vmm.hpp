#ifndef ALKOS_KERNEL_INCLUDE_MEM_VMM_VIRTUAL_MEMORY_MANAGER_HPP_
#define ALKOS_KERNEL_INCLUDE_MEM_VMM_VIRTUAL_MEMORY_MANAGER_HPP_

#include <extensions/types.hpp>

namespace mem
{

class VirtualMemoryManager
{
    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    explicit VirtualMemoryManager(u64 page_table_phys_addr) noexcept;

    // ------------------------------
    // Class interaction
    // ------------------------------

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    u64 page_table_phys_addr_;
};

}  // namespace mem

#endif  // ALKOS_KERNEL_INCLUDE_MEM_VMM_VIRTUAL_MEMORY_MANAGER_HPP_
