#ifndef ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEMORY_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEMORY_HPP_

#include <extensions/expected.hpp>
#include <extensions/type_traits.hpp>
#include <mem/phys_ptr.hpp>
#include <mem/pmm.hpp>
#include <mem/virt_ptr.hpp>
#include <mem/vmm.hpp>

#include "hal/mem/pmm.hpp"
#include "mem/page_map.hpp"

namespace arch
{

template <class PMM>
    requires std::is_base_of_v<PhysicalMemoryManagerABI, PMM>
class VirtualMemoryManager : public VirtualMemoryManagerABI
{
    public:
    static constexpr u64 kNoFlags = 0;
    struct MapOnePageTag {
    };

    //==============================================================================
    // ABI
    //==============================================================================

    void Alloc(const VirtualPtr<byte> vaddr, const u64 size, const u64 flags = kNoFlags);

    template <PageSizeTag kPageSizeTag = PageSizeTag::k4Kb>
    void Map(
        const VirtualPtr<byte> vaddr, const PhysicalPtr<byte> paddr, const u64 size,
        const u64 flags = kNoFlags
    );

    template <PageSizeTag kPageSizeTag = PageSizeTag::k4Kb>
    void Map(
        MapOnePageTag, const VirtualPtr<byte> vaddr, const PhysicalPtr<byte> paddr,
        const u64 flags = kNoFlags
    );

    PhysicalPtr<PageMapTable<4>> GetPml4Table() { return pm_table_4_; }

    private:
    //==============================================================================
    // Private Methods
    //==============================================================================

    template <size_t kLevel>
    FORCE_INLINE_F void EnsurePMEntryPresent(PageMapEntry<kLevel>& pme);

    template <size_t kLevel>
    FORCE_INLINE_F PhysicalPtr<PageMapTable<kLevel - 1>> AllocNextLevelTable();

    template <size_t kLevel>
    FORCE_INLINE_F u64 PmeIdx(u64 addr);

    //==============================================================================
    // Private Fields
    //==============================================================================

    PhysicalMemoryManager& pmm_;
    PhysicalPtr<PageMapTable<4>> pm_table_4_;
};

};
}

#endif  // ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEMORY_HPP_
