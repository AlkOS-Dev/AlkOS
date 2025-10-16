#include <extensions/debug.hpp>

#include "mem/phys/ptr.hpp"
#include "mem/virt/vmm.hpp"
#include "mem/virt/addr_space.hpp"

namespace mem
{

VirtualMemoryManager::VirtualMemoryManager(AddressSpace as) noexcept : current_as_{PhysicalPtr<void>()}
{
    TRACE_INFO("VirtualMemoryManager::VirtualMemoryManager()");
}

}  // namespace mem
