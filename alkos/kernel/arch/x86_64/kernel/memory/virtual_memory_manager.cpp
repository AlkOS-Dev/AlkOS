#include <memory.h>
#include <memory/page_tables_layout.hpp>

#include "memory/physical_memory_manager.hpp"
#include "virtual_memory_manager.hpp"

namespace memory
{

PML4_t& VirtualMemoryManager::GetPml4Table() { return pml4_; }

void VirtualMemoryManager::Allocate(u64 virtual_address, u64 flags)
{
    PhysicalMemoryManager& physical_memory_manager = ::PhysicalMemoryManager::Get();

    R_ASSERT(IsAligned(virtual_address, 1 << GetPageAddressShift<PageSize::Page4k>()));

    // Ensure PML4 entry points to the correct PDPT
    PML4_t& pml4_table = GetPml4Table();
    auto& pml4_entry   = pml4_table[GetPmlIndex<4>(virtual_address)];
    EnsureEntryPresent<4>(pml4_entry, physical_memory_manager);
    auto& pml3_table = GetChildPageDirectory<4>(pml4_entry);
    auto& pml3_entry = pml3_table[GetPmlIndex<3>(virtual_address)];
    EnsureEntryPresent<3>(pml3_entry, physical_memory_manager);
    auto& pml2_table = GetChildPageDirectory<3>(pml3_entry);
    auto& pml2_entry = pml2_table[GetPmlIndex<2>(virtual_address)];
    EnsureEntryPresent<2>(pml2_entry, physical_memory_manager);
    auto& pml1_table    = GetChildPageDirectory<2>(pml2_entry);
    auto& pml1_entry    = pml1_table[GetPmlIndex<1>(virtual_address)];
    pml1_entry.present  = 1;
    pml1_entry.writable = 1;
    pml1_entry.frame =
        physical_memory_manager.Allocate() >> GetPageAddressShift<PageSize::Page4k>();
    u64* entry = reinterpret_cast<u64*>(&pml1_entry);
    *entry |= flags;
}
void VirtualMemoryManager::Free(u64 virtual_address) {}

}  // namespace memory
