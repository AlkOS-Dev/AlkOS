#include "hal/impl/mmu.hpp"

#include <modules/memory.hpp>
#include "mem/page_map.hpp"

using namespace arch;
using namespace Mem;

u64 Mmu::ToArchFlags(PageFlags flags)
{
    u64 arch_flags = 0;
    if (flags.Present) {
        arch_flags |= kPresentBit;
    }
    if (flags.Writable) {
        arch_flags |= kWriteBit;
    }
    if (flags.UserAccessible) {
        arch_flags |= kUserAccessibleBit;
    }
    if (flags.WriteThrough) {
        arch_flags |= kWriteThroughCachingBit;
    }
    if (flags.CacheDisable) {
        arch_flags |= kDisableCacheBit;
    }
    if (flags.Global) {
        arch_flags |= kGlobalBit;
    }
    if (flags.NoExecute) {
        arch_flags |= kNoExecuteBit;
    }
    return arch_flags;
}

Expected<void, MemError> Mmu::Map(
    VPtr<AddressSpace> as, VPtr<void> vaddr, PPtr<void> paddr, PageFlags flags
)
{
    auto res = WalkToEntry<1>(as, vaddr, true);
    UNEXPETED_RET_IF_ERR(res);

    auto pme_l1 = *res;
    if (pme_l1->IsPresent()) {
        // Page is already mapped, unmap first
        return Unexpected(MemError::InvalidArgument);
    }

    pme_l1->SetFrameAddress(paddr, ToArchFlags(flags));

    MemoryModule::Get().GetTlb().InvalidatePage(vaddr);

    return {};
}

Expected<void, MemError> Mmu::UnMap(VPtr<AddressSpace> as, VPtr<void> vaddr)
{
    auto res = WalkToEntry<1>(as, vaddr, false);
    UNEXPETED_RET_IF_ERR(res);

    auto pme_l1 = *res;
    if (!pme_l1->IsPresent()) {
        return {};  // Already unmapped
    }

    // Clear the entry
    *reinterpret_cast<u64 *>(pme_l1) = 0;

    MemoryModule::Get().GetTlb().InvalidatePage(vaddr);

    return {};
}

Expected<PPtr<void>, MemError> Mmu::Translate(VPtr<AddressSpace> as, VPtr<void> vaddr)
{
    auto res = WalkToEntry<1>(as, vaddr, false);
    UNEXPETED_RET_IF_ERR(res);

    auto pme_l1 = *res;
    if (!pme_l1->IsPresent()) {
        return Unexpected(MemError::InvalidArgument);
    }

    return pme_l1->GetFrameAddress();
}
