#ifndef KERNEL_ARCH_X86_64_BOOT_LIB_MEM_VMM_HPP_
#define KERNEL_ARCH_X86_64_BOOT_LIB_MEM_VMM_HPP_

#include <memory.hpp>

#include "mem/error.hpp"
#include "mem/page_map.hpp"
#include "mem/physical_ptr.hpp"
#include "mem/pmm.hpp"

class VirtualMemoryManager
{
    static constexpr u64 kNoFlags = 0;
    struct MapOnePageTag {
    };

    public:
    struct VmmState {
        u64 pml_4_table_phys_addr;
    };

    //==============================================================================
    // Class Creation & Destruction
    //==============================================================================

    explicit VirtualMemoryManager(PhysicalMemoryManager &pmm);
    explicit VirtualMemoryManager(PhysicalMemoryManager &pmm, const VmmState &vmm_state)
        : pmm_{pmm}, pm_table_4_{vmm_state.pml_4_table_phys_addr}
    {
    }

    //==============================================================================
    // Public Methods
    //==============================================================================

    template <decltype(auto) AllocFunc = &PhysicalMemoryManager::Alloc>
    void Alloc(const u64 virt_addr, const u64 size, const u64 flags = kNoFlags);

    template <
        PageSizeTag kPageSizeTag = PageSizeTag::k4Kb,
        decltype(auto) AllocFunc = &PhysicalMemoryManager::Alloc>
    void Map(const u64 virt_addr, const u64 phys_addr, const u64 size, const u64 flags = kNoFlags);

    template <
        PageSizeTag kPageSizeTag = PageSizeTag::k4Kb,
        decltype(auto) AllocFunc = &PhysicalMemoryManager::Alloc>
    void Map(MapOnePageTag, const u64 virt_addr, const u64 phys_addr, const u64 flags = kNoFlags);

    PhysicalPtr<PageMapTable<4>> GetPml4Table() { return pm_table_4_; }
    VmmState GetState() { return VmmState{.pml_4_table_phys_addr = GetPml4Table().Value()}; }

    private:
    //==============================================================================
    // Private Methods
    //==============================================================================

    template <size_t kLevel, decltype(auto) AllocFunc = &PhysicalMemoryManager::Alloc>
    void EnsurePMEntryPresent(PageMapEntry<kLevel> &pme);

    template <size_t kLevel, decltype(auto) AllocFunc = &PhysicalMemoryManager::Alloc>
    PhysicalPtr<PageMapTable<kLevel - 1>> AllocNextLevelTable();

    template <size_t kLevel>
    u64 PmeIdx(u64 addr);

    //==============================================================================
    // Private Fields
    //==============================================================================

    PhysicalMemoryManager &pmm_;
    PhysicalPtr<PageMapTable<4>> pm_table_4_;
};

#include "mem/vmm.tpp"

#endif  // KERNEL_ARCH_X86_64_BOOT_LIB_MEM_VMM_HPP_
