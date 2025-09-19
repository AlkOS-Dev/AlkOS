#ifndef ALKOS_BOOT_LIB_MEM_VMM_TPP_
#define ALKOS_BOOT_LIB_MEM_VMM_TPP_

#include "mem/vmm.hpp"

//==============================================================================
// Public Methods
//==============================================================================

template <decltype(auto) AllocFunc>
void VirtualMemoryManager::Alloc(const u64 virt_addr, const u64 size, const u64 flags)
{
    const u64 stride  = PageSize<PageSizeTag::k4Kb>();
    const u64 al_size = AlignUp(size, stride);

    for (u64 offset = 0; offset < al_size; offset += stride) {
        const auto phys_page_res = (pmm_.*AllocFunc)();
        ASSERT_TRUE(phys_page_res, "Failed to allocate physical page");
        const auto phys_page = *phys_page_res;

        Map<PageSizeTag::k4Kb, AllocFunc>(
            MapOnePageTag{}, virt_addr + offset, phys_page.Value(), flags
        );
    }
}

template <PageSizeTag kPageSizeTag, decltype(auto) AllocFunc>
void VirtualMemoryManager::Map(
    const u64 virt_addr, const u64 phys_addr, const u64 size, const u64 flags
)
{
    const u64 stride = PageSize<kPageSizeTag>();

    for (u64 offset = 0; offset < AlignUp(size, stride); offset += stride) {
        Map<kPageSizeTag, AllocFunc>(
            MapOnePageTag{}, virt_addr + offset, phys_addr + offset, flags
        );
    }
}

template <PageSizeTag kPageSizeTag, decltype(auto) AllocFunc>
void VirtualMemoryManager::Map(
    MapOnePageTag, const u64 virt_addr, const u64 phys_addr, const u64 flags
)
{
    static constexpr u64 kDefaultFlags = kPresentBit | kWriteBit | kUserAccessibleBit;

    // !!!
    // Note: Has to be understood that this whole function
    // works only because of the identity mapping
    // !!!

    PageMapTable<4>& pm_table_4 = *pm_table_4_;

    PageMapEntry<4>& pme_4 = pm_table_4[PmeIdx<4>(virt_addr)];
    EnsurePMEntryPresent<4, AllocFunc>(pme_4);

    if constexpr (kPageSizeTag == PageSizeTag::k1Gb) {
        ASSERT_TRUE((virt_addr & kBitMaskRight<u64, 30>) == 0, "1GB pages must be 1GB-aligned");
        ASSERT_TRUE((phys_addr & kBitMaskRight<u64, 30>) == 0, "1GB pages must be 1GB-aligned");

        PageMapEntry<3>& pme_3 = (*pme_4.GetNextLevelTable())[PmeIdx<3>(virt_addr)];
        PageMapEntry<3, kHugePage>& pme_3_1gb =
            reinterpret_cast<PageMapEntry<3, kHugePage>&>(pme_3);
        ASSERT_FALSE(pme_3.present);

        pme_3_1gb.SetFrameAddress(
            PhysicalPtr<void>{phys_addr}, flags | kDefaultFlags | kHugePageBit
        );
        return;
    }

    PageMapEntry<3>& pme_3 = (*pme_4.GetNextLevelTable())[PmeIdx<3>(virt_addr)];
    EnsurePMEntryPresent<3, AllocFunc>(pme_3);

    if constexpr (kPageSizeTag == PageSizeTag::k2Mb) {
        ASSERT_TRUE((virt_addr & kBitMaskRight<u64, 21>) == 0, "2MB pages must be 2MB-aligned");
        ASSERT_TRUE((phys_addr & kBitMaskRight<u64, 21>) == 0, "2MB pages must be 2MB-aligned");

        PageMapEntry<2>& pme_2 = (*pme_3.GetNextLevelTable())[PmeIdx<2>(virt_addr)];
        PageMapEntry<2, kHugePage>& pme_2_2mb =
            reinterpret_cast<PageMapEntry<2, kHugePage>&>(pme_2);
        ASSERT_FALSE(pme_2.present);

        pme_2_2mb.SetFrameAddress(
            PhysicalPtr<void>{phys_addr}, flags | kDefaultFlags | kHugePageBit
        );
        return;
    }

    PageMapEntry<2>& pme_2 = (*pme_3.GetNextLevelTable())[PmeIdx<2>(virt_addr)];
    EnsurePMEntryPresent<2, AllocFunc>(pme_2);

    PageMapEntry<1>& pme_1 = (*pme_2.GetNextLevelTable())[PmeIdx<1>(virt_addr)];

    ASSERT_FALSE(pme_1.present);
    ASSERT_TRUE((virt_addr & kBitMaskRight<u64, 12>) == 0, "4KB pages must be 4KB-aligned");

    pme_1.SetFrameAddress(PhysicalPtr<void>{phys_addr}, flags | kDefaultFlags);
}

//==============================================================================
// Private Methods
//==============================================================================

template <size_t kLevel, decltype(auto) AllocFunc>

FORCE_INLINE_F void VirtualMemoryManager::EnsurePMEntryPresent(PageMapEntry<kLevel>& pme)
{
    if (!pme.present) {
        auto new_table_ptr = AllocNextLevelTable<kLevel, AllocFunc>();
        pme.SetNextLevelTable(new_table_ptr, kPresentBit | kWriteBit | kUserAccessibleBit);
    }
}

template <size_t kLevel, decltype(auto) AllocFunc>
FORCE_INLINE_F PhysicalPtr<PageMapTable<kLevel - 1>> VirtualMemoryManager::AllocNextLevelTable()
{
    auto free_page_res = (pmm_.*AllocFunc)();
    R_ASSERT_TRUE(free_page_res.has_value(), "Allocation function returned unexpected error");

    PhysicalPtr<void> free_page = *free_page_res;
    memset(free_page.ValuePtr(), 0, sizeof(PageMapTable<kLevel - 1>));

    PhysicalPtr<PageMapTable<kLevel - 1>> pml_ptr(free_page);
    return pml_ptr;
}

template <size_t kLevel>
FORCE_INLINE_F u64 VirtualMemoryManager::PmeIdx(u64 addr)
{
    static_assert(kLevel > 0);
    static_assert(kLevel <= 4);
    static constexpr u32 kIndexMask = kBitMaskRight<u32, 9>;
    return (addr >> (12 + (kLevel - 1) * 9)) & kIndexMask;
}

#endif  // ALKOS_BOOT_LIB_MEM_VMM_TPP_
