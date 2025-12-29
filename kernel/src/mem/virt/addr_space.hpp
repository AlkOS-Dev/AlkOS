#ifndef KERNEL_SRC_MEM_VIRT_ADDR_SPACE_HPP_
#define KERNEL_SRC_MEM_VIRT_ADDR_SPACE_HPP_

#include <expected.hpp>
#include <types.hpp>

#include "hal/interrupt_params.hpp"
#include "hal/spinlock.hpp"
#include "interrupts/interrupt_types.hpp"
#include "mem/error.hpp"
#include "mem/types.hpp"
#include "mem/virt/addr_space_iterator.hpp"
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

class AddressSpace
{
    public:
    explicit AddressSpace();

    expected<void, MemError> InitUser(BuddyPmm &pmm, hal::Mmu &mmu, PageMetaTable &pmt);
    expected<void, MemError> InitKernel(
        const PPtr<void> kernel_root, BuddyPmm &pmm, hal::Mmu &mmu, PageMetaTable &pmt
    );

    ~AddressSpace();
    AddressSpace(const AddressSpace &)            = delete;
    AddressSpace &operator=(const AddressSpace &) = delete;

    PPtr<void> PageTableRoot() const { return page_table_root_; }

    // Iterator
    AddrSpIt begin() const { return AddrSpIt(area_list_head_); }
    AddrSpIt end() const { return AddrSpIt(nullptr); }

    private:
    // This is orchestrated in VMM (For proper TLB management)
    expected<void, MemError> AddArea(VMemArea vma);
    expected<void, MemError> RmArea(VPtr<void> ptr);

    // Helpers
    expected<VPtr<VMemArea>, MemError> FindArea(VPtr<void> ptr);
    bool IsAddrInArea(VPtr<VMemArea> vma, VPtr<void> ptr);
    bool AreasOverlap(VPtr<VMemArea> a, VPtr<VMemArea> b);

    // Fields
    PPtr<void> page_table_root_;
    bool owns_page_table_root_;
    VPtr<VMemArea> area_list_head_;
    hal::Spinlock area_list_lock_;

    // Dependencies
    KernelMmuContext *ctx_;
    BuddyPmm *pmm_;
    hal::Mmu *mmu_;
    Mem::PageMetaTable *pmt_;

    // Friends
    friend VirtualMemoryManager;
    friend void PageFaultHandler(intr::LitExcEntry &entry, hal::ExceptionData *data);

    public:
};
using AddrSp = AddressSpace;

}  // namespace Mem

#endif  // KERNEL_SRC_MEM_VIRT_ADDR_SPACE_HPP_
