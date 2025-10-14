#include "extensions/debug.hpp"
#include "mem/vmm/virtual_memory_manager.hpp"

namespace mem
{

VirtualMemoryManager::VirtualMemoryManager(const u64 page_table_phys_addr) noexcept
    : page_table_phys_addr_(page_table_phys_addr)
{
    TRACE_INFO("VirtualMemoryManager::VirtualMemoryManager()");
}

}  // namespace mem
