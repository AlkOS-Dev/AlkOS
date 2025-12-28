#include "hal/impl/mmu.hpp"
#include <macros.hpp>
#include "cpu/control_registers.hpp"
#include "mem/page_map.hpp"

namespace arch
{

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

void Mmu::SwitchRoot(Mem::PPtr<void> root)
{
    cpu::Cr3 cr3{};
    cr3.PageMapLevel4Address = Mem::PtrToUptr(root) >> 12;
    cpu::SetCR(cr3);
}

expected<void, MemError> Mmu::SetPageFlags(
    Mem::PPtr<void> root, Mem::VPtr<void> vaddr, PageFlags flags
)
{
    // Walk to leaf without allocating
    auto *pml4  = reinterpret_cast<PageMapTable<4> *>(Mem::PhysToVirt(root));
    auto &pml4e = (*pml4)[PmeIdx<4>(vaddr)];
    RET_UNEXPECTED_IF(!pml4e.IsPresent(), MemError::NotFound);

    auto *pdpt  = reinterpret_cast<PageMapTable<3> *>(Mem::PhysToVirt(pml4e.GetNextLevelTable()));
    auto &pdpte = (*pdpt)[PmeIdx<3>(vaddr)];
    RET_UNEXPECTED_IF(!pdpte.IsPresent(), MemError::NotFound);
    RET_UNEXPECTED_IF(pdpte.IsHuge(), MemError::InvalidArgument);  // TODO: Support huge pages

    auto *pd  = reinterpret_cast<PageMapTable<2> *>(Mem::PhysToVirt(pdpte.GetNextLevelTable()));
    auto &pde = (*pd)[PmeIdx<2>(vaddr)];
    RET_UNEXPECTED_IF(!pde.IsPresent(), MemError::NotFound);
    RET_UNEXPECTED_IF(pde.IsHuge(), MemError::InvalidArgument);

    auto *pt  = reinterpret_cast<PageMapTable<1> *>(Mem::PhysToVirt(pde.GetNextLevelTable()));
    auto &pte = (*pt)[PmeIdx<1>(vaddr)];
    RET_UNEXPECTED_IF(!pte.IsPresent(), MemError::NotFound);

    // Update flags
    auto paddr = pte.GetFrameAddress();
    pte.SetFrameAddress(paddr, ToArchFlags(flags));

    return {};
}

}  // namespace arch
