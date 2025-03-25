#ifndef ALKOS_ALKOS_KERNEL_ARCH_X86_64_KERNEL_MEMORY_MANAGEMENT_VIRTUAL_MEMORY_MANAGER_TPP_
#define ALKOS_ALKOS_KERNEL_ARCH_X86_64_KERNEL_MEMORY_MANAGEMENT_VIRTUAL_MEMORY_MANAGER_TPP_

#include <memory.h>
#include <memory/page_tables_layout.hpp>
#include "memory/physical_memory_manager.hpp"
#include "memory/virtual_memory_manager.hpp"

namespace memory
{

template <u8 level>
u16 VirtualMemoryManager::GetPmlIndex(u64 virtual_address) const
{
    static const u16 kIndexMask = 0x1FF;
    return (virtual_address >> GetIndexShift<level>()) & kIndexMask;
}

template <u8 level>
constexpr u8 VirtualMemoryManager::GetIndexShift() const
{
    switch (level) {
        case 4:
            return 39;
        case 3:
            return 30;
        case 2:
            return 21;
        case 1:
            return 12;
        default: {
            R_FAIL_ALWAYS("Invalid level.");
            return 0;
        }
    }
}

template <PageSize page_size>
constexpr u8 VirtualMemoryManager::GetPageAddressShift() const
{
    switch (page_size) {
        case PageSize::Page4k:
            return 12;
        case PageSize::Page2M:
            return 21;
        case PageSize::Page1G:
            return 30;
        default: {
            R_FAIL_ALWAYS("Invalid page size.");
            return 0;
        }
    }
}

template <u8 entry_level>
PageTableTypeGetter<entry_level - 1>::Type& VirtualMemoryManager::GetChildPageDirectory(
    PageEntryTypeGetter<entry_level>::Type& entry
)
{
    AssertDescendable<entry_level>();

    return *reinterpret_cast<typename PageTableTypeGetter<entry_level - 1>::Type*>(
        entry.frame << kAddressOffset
    );
}

template <u8 entry_level>
void VirtualMemoryManager::EnsureEntryPresent(
    PageEntryTypeGetter<entry_level>::Type& entry, TableAllocatorConcept auto& allocator
)
{
    AssertDescendable<entry_level>();

    if (!entry.present) {
        uintptr_t table = allocator.Allocate();
        memset(
            reinterpret_cast<void*>(table), 0,
            sizeof(typename PageTableTypeGetter<entry_level - 1>::Type)
        );
        entry.frame    = reinterpret_cast<u64>(table) >> kAddressOffset;
        entry.present  = 1;
        entry.writable = 1;
    }
}

// TODO : Think of a sane way to create a:
// - a) Variant that returns the last valid entry (non-zero) in the page table and the level it was
// found at.
// - b) Variant that allocates a new page table entry if it doesn't exist.
// template <u8 level_end>
// PageEntryTypeGetter<level_end>::Type& VirtualMemoryManager::WalkPageTables(u64 virtual_address)
//{
//    static_assert(level_end > 0, "Level must be greater than 0.");
//    static_assert(level_end <= 4, "Level must be less than to 4.");
//
//    // TODO: Use a compile time unrolled template loop to walk the page tables.
//    PhysicalMemoryManager &physical_memory_manager = ::PhysicalMemoryManager::Get();
//
//    auto& pml4_table = GetPml4Table();
//    auto& pml4_entry = pml4_table[GetPmlIndex<4>(virtual_address)];
//    if constexpr (level_end == 4) {
//        return pml4_entry;
//    }
//    auto& pml3_table = GetChildPageDirectory<4>(pml4_entry);
//    auto& pml3_entry = pml3_table[GetPmlIndex<3>(virtual_address)];
//    if constexpr (level_end == 3) {
//        return pml3_entry;
//    }
//    auto& pml2_table = GetChildPageDirectory<3>(pml3_entry);
//    auto& pml2_entry = pml2_table[GetPmlIndex<2>(virtual_address)];
//    if constexpr (level_end == 2) {
//        return pml2_entry;
//    }
//    auto& pml1_table = GetChildPageDirectory<2>(pml2_entry);
//    auto& pml1_entry = pml1_table[GetPmlIndex<1>(virtual_address)];
//    return pml1_entry;
//}

template <u8 level>
constexpr void VirtualMemoryManager::AssertDescendable()
{
    static_assert(level > 0, "Level must be greater than 0.");
    static_assert(level <= 4, "Level must be less than or equal to 4.");
}

}  // namespace memory

#endif ALKOS_ALKOS_KERNEL_ARCH_X86_64_KERNEL_MEMORY_MANAGEMENT_VIRTUAL_MEMORY_MANAGER_TPP_
