#include <extensions/debug.hpp>

#include "mem/heap.hpp"
#include "mem/phys/ptr.hpp"
#include "mem/virt/addr_space.hpp"
#include "mem/virt/vmm.hpp"

namespace mem
{

VirtualMemoryManager::VirtualMemoryManager() noexcept
{
    TRACE_INFO("VirtualMemoryManager::VirtualMemoryManager()");
}

Expected<VirtualPtr<AddressSpace>, MemError> VirtualMemoryManager::CreateAddrSpace()
{
    return KMalloc<AddressSpace>();
}

Expected<void, MemError> VirtualMemoryManager::DestroyAddrSpace(VirtualPtr<AddressSpace> as)
{
    // Later invalidate TLB etc
    return KFree(as);
}

void VirtualMemoryManager::SwitchAddrSpace(VirtualPtr<AddressSpace> as)
{
    // TODO
}

}  // namespace mem
