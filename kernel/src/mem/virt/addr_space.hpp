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

struct GapInfo {
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

    void Lock() { area_list_lock_.Lock(); }
    void Unlock() { area_list_lock_.Unlock(); }

    private:
    // Store pointers to polymorphic VMemArea objects
    using AddrSpIt = data_structures::DoubleLinkedList<VMemArea *>::ConstIterator;

    // Iterator
    AddrSpIt begin() const { return area_list_.begin(); }
    AddrSpIt end() const { return area_list_.end(); }

    // Takes ownership of vma pointer
    expected<void, MemError> AddArea(VMemArea *vma);

    expected<TlbHint, MemError> RmArea(VPtr<void> ptr);
    expected<TlbHint, MemError> UpdateAreaFlags(VPtr<void> ptr, VirtualMemAreaFlags flags);
    expected<GapInfo, MemError> FindGap(
        size_t size, VPtr<void> start = nullptr, VPtr<void> end = nullptr
    );

    // Helpers
    using AddrSpMutIt = data_structures::DoubleLinkedList<VMemArea *>::Iterator;
    expected<AddrSpMutIt, MemError> FindAreaLocked(VPtr<void> ptr);
    expected<VMemArea *, MemError> FindArea(VPtr<void> ptr);
    bool IsAddrInArea(const VMemArea *vma, VPtr<void> ptr);
    bool AreasOverlap(const VMemArea *a, const VMemArea *b);

    // Fields
    PPtr<void> page_table_root_;
    bool owns_page_table_root_;

    /// @brief Sorted list of pointers to VMA objects. We own these objects.
    /// @note The list is sorted by the start address of the VMA objects.
    data_structures::DoubleLinkedList<VMemArea *> area_list_;
    hal::Spinlock area_list_lock_;

    // Dependencies
    KernelMmuContext *ctx_;
    hal::Mmu *mmu_;

    // Friends
    friend class VirtualMemoryManager;
    friend Sched::Thread *PageFaultHandler(intr::LitExcEntry &entry, hal::ExceptionData *data);

    public:
};

using AddrSp = AddressSpace;

}  // namespace Mem

#endif  // KERNEL_SRC_MEM_VIRT_ADDR_SPACE_HPP_
