#ifndef KERNEL_ARCH_X86_64_SRC_HAL_IMPL_MMU_HPP_
#define KERNEL_ARCH_X86_64_SRC_HAL_IMPL_MMU_HPP_

#include <expected.hpp>
#include <hal/api/mmu.hpp>
#include <mem/types.hpp>

namespace arch
{

using std::expected;
using std::unexpected;

class Mmu : public MmuAPI
{
    public:
    template <MmuContext Context>
    expected<void, Mem::MemError> Map(
        Context &ctx, Mem::PPtr<void> root, Mem::VPtr<void> vaddr, Mem::PPtr<void> paddr,
        PageFlags flags
    );

    template <MmuContext Context>
    void Unmap(Context &ctx, Mem::PPtr<void> root, Mem::VPtr<void> vaddr);

    template <MmuContext Context>
    void ClearUserMappings(Context &ctx, Mem::PPtr<void> root);

    template <MmuContext Context>
    expected<Mem::PPtr<void>, Mem::MemError> Translate(
        Context &ctx, Mem::PPtr<void> root, Mem::VPtr<void> vaddr
    );

    expected<void, Mem::MemError> SetPageFlags(
        Mem::PPtr<void> root, Mem::VPtr<void> vaddr, PageFlags flags
    );

    void SwitchRoot(Mem::PPtr<void> root);

    void CopyKernelSpace(Mem::PPtr<void> dst_root, Mem::PPtr<void> kernel_root);

    template <TableVisitor Visitor>
    void VisitTables(Mem::PPtr<void> root, Visitor visitor);

    /**
     * @brief Recursively destroys a page table tree.
     * Used for cleanup operations (e.g. destroying an address space or cleaning lower half).
     */
    template <MmuContext Context>
    void DestroyTable(Context &ctx, Mem::PPtr<void> table, uint8_t level);

    private:
    u64 ToArchFlags(PageFlags flags);

    template <size_t kLevel>
    u64 PmeIdx(Mem::VPtr<void> vaddr);
};

}  // namespace arch

#include "hal/impl/mmu.tpp"

#endif  // KERNEL_ARCH_X86_64_SRC_HAL_IMPL_MMU_HPP_
