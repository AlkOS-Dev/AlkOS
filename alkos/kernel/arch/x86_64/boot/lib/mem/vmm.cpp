#include <extensions/bit.hpp>
#include <extensions/debug.hpp>
#include <extensions/defines.hpp>
#include <extensions/types.hpp>

#include "mem/error.hpp"
#include "mem/page_map.hpp"
#include "mem/physical_ptr.hpp"
#include "mem/vmm.hpp"
#include "sys/panic.hpp"

static PageMapTable<4>& AllocatePml4Table(PhysicalMemoryManager& pmm)
{
    auto free_page_res = pmm.Alloc();
    if (!free_page_res) {
        KernelPanic("VirtualMemoryManager::VirtualMemoryManager(): Failed to allocate PML4");
    }

    TRACE_DEBUG("PML4 allocated at physical address %p", *free_page_res);
    const PhysicalPtr<void> free_page = *free_page_res;
    PhysicalPtr<PageMapTable<4>> pml_ptr(free_page);
    // TODO: Validate that pml4 is below 4GB in order for this memset to work
    // memset(pml_ptr.Value(), 0, sizeof(PageMapTable<4>));
    return *pml_ptr;
}

VirtualMemoryManager::VirtualMemoryManager(PhysicalMemoryManager& pmm)
    : pmm_{pmm}, pml4_{AllocatePml4Table(pmm)}
{
}

void VirtualMemoryManager::Map(u64 virt_addr, u64 phys_addr, u64 flags)
{
    static constexpr u32 kIndexMask = kBitMaskRight<u32, 9>;
    // TODO: Replace with func
    const u32 pme_4_idx = (virt_addr >> 39) & kIndexMask;
    const u32 pme_3_idx = (virt_addr >> 30) & kIndexMask;
    const u32 pme_2_idx = (virt_addr >> 21) & kIndexMask;
    const u32 pme_1_idx = (virt_addr >> 12) & kIndexMask;

    // Ensure PML4 entry points to the correct PDPT
    auto& pml4_entry = pml4_[pme_4_idx];
}
