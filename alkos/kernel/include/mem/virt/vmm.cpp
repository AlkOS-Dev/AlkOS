#include <extensions/debug.hpp>

#include "mem/heap.hpp"
#include "mem/phys/ptr.hpp"
#include "mem/virt/addr_space.hpp"
#include "mem/virt/vmm.hpp"

namespace mem
{

using Vmm = VirtualMemoryManager;
template <typename T>
using VPtr = VirtualPtr<T>;

Vmm::VirtualMemoryManager() noexcept { TRACE_INFO("VirtualMemoryManager::VirtualMemoryManager()"); }

Expected<VirtualPtr<AddressSpace>, MemError> Vmm::CreateAddrSpace()
{
    return KMalloc<AddressSpace>();
}

Expected<void, MemError> Vmm::DestroyAddrSpace(VPtr<AddressSpace> as)
{
    // Later invalidate TLB etc
    KFree(as);
    return {};
}

void Vmm::SwitchAddrSpace(VPtr<AddressSpace> as)
{
    // TODO
}

}  // namespace mem
