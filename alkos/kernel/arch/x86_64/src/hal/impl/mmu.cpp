#include "hal/impl/mmu.hpp"

#include "mem/page_map.hpp"

using namespace arch;
using namespace Mem;

u64 Mmu::ToArchFlags(PageFlags flags)
{
    u64 arch_flags = 0;
    return arch_flags;
}

Expected<void, MemError> Mmu::Map(
    VPtr<AddressSpace> as, VPtr<void> vaddr, PPtr<void> paddr, PageFlags flags
)
{
    auto res = WalkToEntry<1>(as, vaddr, true);
    if (!res) {
        return Unexpected(res.error());
    }

    auto pme_l1 = *res;
    if (pme_l1->IsPresent()) {
        // Page is already mapped, unmap first
        return Unexpected(MemError::InvalidArgument);
    }

    pme_l1->SetFrameAddress(paddr, ToArchFlags(flags));

    // Invalidate the TLB for this page
    // TODO
    // HardwareModule::Get().GetTlb().InvalidatePage(vaddr);

    return {};
}

Expected<void, MemError> Mmu::UnMap(VPtr<AddressSpace> as, VPtr<void> vaddr)
{
    auto res = WalkToEntry<1>(as, vaddr, false);
    if (!res) {
        return Unexpected(res.error());
    }

    auto pme_l1 = *res;
    if (!pme_l1->IsPresent()) {
        return {};  // Already unmapped
    }

    // Clear the entry
    *reinterpret_cast<u64 *>(pme_l1) = 0;

    // Invalidate the TLB for this page
    // HardwareModule::Get().GetTlb().InvalidatePage(vaddr);

    return {};
}
