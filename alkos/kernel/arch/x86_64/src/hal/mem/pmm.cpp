#include "hal/mem/pmm.hpp"
#include <mem/phys_ptr.hpp>

using namespace arch;

class PrimitivePmm : PhysicalMemoryManagerABI
{
    PrimitivePmm(PhysicalMemoryManager::InitData idata) {}

    std::expected<PhysicalPtr<Page>, MemError> Alloc(AllocationRequest /* req */ = {})
    {
        return PhysicalPtr<Page>(static_cast<uintptr_t>(0));
    }
    void Free(PhysicalPtr<byte> /*page*/) { return; /* No Free */ }
};

void PhysicalMemoryManager::Init(PhysicalMemoryManager::InitData idata) { return; }
