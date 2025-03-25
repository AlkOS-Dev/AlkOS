#include <memory.h>
#include <memory/page_tables_layout.hpp>

#include "memory/physical_memory_manager.hpp"
#include "virtual_memory_manager.hpp"

namespace memory
{

PML4_t &VirtualMemoryManager::GetPml4Table() { return pml4_; }

void VirtualMemoryManager::Allocate(u64 virtual_address, u64 flags)
{
    static constexpr u32 kIndexMask = 0x1FF;
    static constexpr i32 kPageShift = 12;

    PhysicalMemoryManager &physical_memory_manager = ::PhysicalMemoryManager::Get();

    R_ASSERT(IsAligned(virtual_address, 1 << kPageShift));

    // Calculate the indexes for each level of the page table
    const u32 pml4_index = (virtual_address >> 39) & kIndexMask;
    const u32 pml3_index = (virtual_address >> 30) & kIndexMask;
    const u32 pml2_index = (virtual_address >> 21) & kIndexMask;
    const u32 pml1_index = (virtual_address >> 12) & kIndexMask;

    // Ensure PML4 entry points to the correct PDPT
    PML4_t &pml4_table = GetPml4Table();

    auto *pml4_entry = reinterpret_cast<PML4Entry_t *>(&(pml4_table)[pml4_index]);
    if (!pml4_entry->present) {
        // Allocate a new PML3 table
        uintptr_t pml3_table = physical_memory_manager.Allocate();
        memset(reinterpret_cast<void *>(pml3_table), 0, sizeof(PML3_t));
        pml4_entry->frame    = reinterpret_cast<u64>(pml3_table) >> kAddressOffset;
        pml4_entry->present  = 1;
        pml4_entry->writable = 1;
    }

    auto pml3_table = reinterpret_cast<PML3Entry_t *>(pml4_entry->frame << kAddressOffset);

    if (!pml3_table[pml3_index].present) {
        // Allocate a new PML2 table
        uintptr_t pml2_table = physical_memory_manager.Allocate();
        memset(reinterpret_cast<void *>(pml2_table), 0, sizeof(PML2_t));
        pml3_table[pml3_index].frame    = reinterpret_cast<u64>(pml2_table) >> kAddressOffset;
        pml3_table[pml3_index].present  = 1;
        pml3_table[pml3_index].writable = 1;
    }

    auto pml2_table =
        reinterpret_cast<PML2Entry_t *>(pml3_table[pml3_index].frame << kAddressOffset);

    if (!pml2_table[pml2_index].present) {
        // Allocate a new PML1 table
        uintptr_t pml1_table = physical_memory_manager.Allocate();
        memset(reinterpret_cast<void *>(pml1_table), 0, sizeof(PML1_t));
        pml2_table[pml2_index].frame    = reinterpret_cast<u64>(pml1_table) >> kAddressOffset;
        pml2_table[pml2_index].present  = 1;
        pml2_table[pml2_index].writable = 1;
    }

    auto p1_entry = reinterpret_cast<PML1Entry_t *>(pml2_table[pml2_index].frame << kAddressOffset);
    p1_entry[pml1_index].present  = 1;
    p1_entry[pml1_index].writable = 1;
    p1_entry[pml1_index].frame    = physical_memory_manager.Allocate() >> kPageShift;
    u64 *entry                    = reinterpret_cast<u64 *>(&p1_entry[pml1_index]);
    *entry |= flags;
}

}  // namespace memory
