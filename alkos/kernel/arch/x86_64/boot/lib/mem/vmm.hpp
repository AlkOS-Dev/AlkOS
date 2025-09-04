#ifndef ALKOS_BOOT_LIB_MEM_VMM_HPP_
#define ALKOS_BOOT_LIB_MEM_VMM_HPP_

#include <extensions/memory.hpp>

#include "mem/error.hpp"
#include "mem/page_map.hpp"
#include "mem/pmm.hpp"

class VirtualMemoryManager
{
    static constexpr u64 kNoFlags = 0;

    public:
    //==============================================================================
    // Class Creation & Destruction
    //==============================================================================

    explicit VirtualMemoryManager(PhysicalMemoryManager& pmm);

    //==============================================================================
    // Public Methods
    //==============================================================================

    void Map(u64 virt_addr, u64 phys_addr, u64 flags = kNoFlags);

    PageMapTable<4>* GetPml4Table() { return std::addressof(pml4_); }

    private:
    //==============================================================================
    // Private Fields
    //==============================================================================

    PageMapTable<4>& pml4_;
    PhysicalMemoryManager& pmm_;
};

#endif  // ALKOS_BOOT_LIB_MEM_VMM_HPP_
