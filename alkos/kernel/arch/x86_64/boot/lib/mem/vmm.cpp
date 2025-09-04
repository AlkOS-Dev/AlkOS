#include <extensions/bit.hpp>
#include <extensions/debug.hpp>
#include <extensions/defines.hpp>
#include <extensions/memory.hpp>
#include <extensions/types.hpp>

#include "mem/error.hpp"
#include "mem/page_map.hpp"
#include "mem/physical_ptr.hpp"
#include "mem/vmm.hpp"
#include "sys/panic.hpp"

static PageMapTable<4>& AllocatePml4Table(PhysicalMemoryManager& pmm)
{
    auto free_page_res = pmm.Alloc32();
    if (!free_page_res) {
        KernelPanic("VirtualMemoryManager::VirtualMemoryManager(): Failed to allocate PML4");
    }

    TRACE_DEBUG("PML4 allocated at physical address %p", *free_page_res);
    PhysicalPtr<void> free_page = *free_page_res;
    PhysicalPtr<PageMapTable<4>> pml_ptr(free_page);
    memset(free_page.ValuePtr(), 0, sizeof(PageMapTable<4>));
    return *pml_ptr;
}

VirtualMemoryManager::VirtualMemoryManager(PhysicalMemoryManager& pmm)
    : pmm_{pmm}, pm_table_4{AllocatePml4Table(pmm)}
{
}
