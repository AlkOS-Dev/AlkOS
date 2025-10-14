#include "mem/vmm/vmm.hpp"
#include "extensions/debug.hpp"
#include "mem/vmm/addr_space.hpp"

namespace mem
{

VirtualMemoryManager::VirtualMemoryManager(AddressSpace as) noexcept : current_as_(as)
{
    TRACE_INFO("VirtualMemoryManager::VirtualMemoryManager()");
}

}  // namespace mem
