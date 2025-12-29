#ifndef KERNEL_SRC_MEM_VIRT_ADDR_SPACE_HPP_
#define KERNEL_SRC_MEM_VIRT_ADDR_SPACE_HPP_

#include <expected.hpp>
#include <types.hpp>

#include <data_structures/linked_list.hpp>
#include "hal/interrupt_params.hpp"
#include "hal/spinlock.hpp"
#include "interrupts/interrupt_types.hpp"
#include "mem/error.hpp"
#include "mem/types.hpp"
#include "mem/virt/area.hpp"

namespace hal
{
class Mmu;
}

namespace Mem
{

struct KernelMmuContext;

using std::expected;
using std::unexpected;

//==============================================================================
// Forward Declarations
//==============================================================================

class VirtualMemoryManager;
class BuddyPmm;
class PageMetaTable;

//==============================================================================
// AddressSpace
//==============================================================================

struct TlbHint {
    VPtr<void> start;
    size_t size;
};

class AddressSpace
{
    public:
    explicit AddressSpace();

    expected<void, MemError> InitUser(KernelMmuContext &ctx, hal::Mmu &mmu);
    expected<void, MemError> InitKernel(
        const PPtr<void> kernel_root, KernelMmuContext &ctx, hal::Mmu &mmu
    );

    ~AddressSpace();
    AddressSpace(const AddressSpace &)            = delete;
    AddressSpace &operator=(const AddressSpace &) = delete;

    PPtr<void> PageTableRoot() const { return page_table_root_; }

    using AddrSpIt = data_structures::DoubleLinkedList<VMemArea>::ConstIterator;

    // Iterator
    AddrSpIt begin() const { return area_list_.begin(); }
    AddrSpIt end() const { return area_list_.end(); }

    private:
    // This is orchestrated in VMM (For proper TLB management)
    expected<void, MemError> AddArea(VMemArea vma);
    expected<TlbHint, MemError> RmArea(VPtr<void> ptr);
    expected<TlbHint, MemError> UpdateAreaFlags(VPtr<void> ptr, VirtualMemAreaFlags flags);

    // Helpers
    using AddrSpMutIt = data_structures::DoubleLinkedList<VMemArea>::Iterator;
    expected<AddrSpMutIt, MemError> FindAreaLocked(VPtr<void> ptr);
    expected<VPtr<VMemArea>, MemError> FindArea(VPtr<void> ptr);
    bool IsAddrInArea(VPtr<VMemArea> vma, VPtr<void> ptr);
    bool AreasOverlap(VPtr<VMemArea> a, VPtr<VMemArea> b);

    // Fields
    PPtr<void> page_table_root_;
    bool owns_page_table_root_;
    data_structures::DoubleLinkedList<VMemArea> area_list_;
    hal::Spinlock area_list_lock_;

    // Dependencies
    KernelMmuContext *ctx_;
    hal::Mmu *mmu_;

    // Friends
    friend VirtualMemoryManager;
    friend void PageFaultHandler(intr::LitExcEntry &entry, hal::ExceptionData *data);

    public:
};
using AddrSp = AddressSpace;

}  // namespace Mem

#endif  // KERNEL_SRC_MEM_VIRT_ADDR_SPACE_HPP_
