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

    template <decltype(auto) AllocFunc = &PhysicalMemoryManager::Alloc>
    void Map(u64 virt_addr, u64 phys_addr, u64 flags = kNoFlags)
    {
        static constexpr u32 kIndexMask    = kBitMaskRight<u32, 9>;
        static constexpr u64 kDefaultFlags = kPresentBit | kWriteBit | kUserAccessibleBit;

        PageMapEntry<4>& pme_4 = pm_table_4[PmeIdx<4>(virt_addr)];
        if (!pme_4.present) {
            auto& new_table = AllocNextLevelTable<4, AllocFunc>();
            pme_4.SetNextLevelTable(new_table, kDefaultFlags);
        }

        PageMapEntry<3>& pme_3 = pme_4.GetNextLevelTable()[PmeIdx<3>(virt_addr)];
        if (!pme_3.present) {
            auto& new_table = AllocNextLevelTable<3, AllocFunc>();
            pme_3.SetNextLevelTable(new_table, kDefaultFlags);
        }

        PageMapEntry<2>& pme_2 = pme_3.GetNextLevelTable()[PmeIdx<2>(virt_addr)];
        if (!pme_2.present) {
            auto& new_table = AllocNextLevelTable<2, AllocFunc>();
            pme_2.SetNextLevelTable(new_table, kDefaultFlags);
        }

        PageMapEntry<1>& pme_1 = pme_2.GetNextLevelTable()[PmeIdx<1>(virt_addr)];

        ASSERT_FALSE(pme_1.present);
        pme_1.SetFrameAddress(PhysicalPtr<void>{phys_addr}, flags | kDefaultFlags);
    }

    PageMapTable<4>* GetPml4Table() { return std::addressof(pm_table_4); }

    private:
    //==============================================================================
    // Private Fields
    //==============================================================================

    template <size_t kLevel, decltype(auto) AllocFunc = &PhysicalMemoryManager::Alloc>
    FORCE_INLINE_F PageMapTable<kLevel - 1>& AllocNextLevelTable()
    {
        auto free_page_res = (pmm_.*AllocFunc)();
        R_ASSERT_TRUE(free_page_res.has_value(), "Allocation function returned unexpected error");

        PhysicalPtr<void> free_page = *free_page_res;
        memset(free_page.ValuePtr(), 0, sizeof(PageMapTable<kLevel - 1>));

        PhysicalPtr<PageMapTable<kLevel - 1>> pml_ptr(free_page);
        return *pml_ptr;
    }

    template <size_t kLevel>
    FORCE_INLINE_F u64 PmeIdx(u64 addr)
    {
        static_assert(kLevel > 0);
        static_assert(kLevel <= 4);
        static constexpr u32 kIndexMask = kBitMaskRight<u32, 9>;
        return (addr >> (12 + (kLevel - 1) * 9)) & kIndexMask;
    }

    PhysicalMemoryManager& pmm_;
    PageMapTable<4>& pm_table_4;
};

#endif  // ALKOS_BOOT_LIB_MEM_VMM_HPP_
