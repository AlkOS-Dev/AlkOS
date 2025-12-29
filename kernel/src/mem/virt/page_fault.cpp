#include "mem/virt/page_fault.hpp"

#include "interrupts/interrupt_types.hpp"

#include "hal/intr_parser.hpp"
#include "hal/panic.hpp"
#include "mem/mmu/contexts.hpp"
#include "mem/phys/mngr/buddy.hpp"
#include "mem/virt/page_fault.hpp"
#include "modules/hardware.hpp"
#include "modules/memory.hpp"
#include "trace_framework.hpp"

namespace Mem
{

void PageFaultHandler(intr::LitExcEntry &, hal::ExceptionData *data)
{
    TRACE_INFO_GENERAL("PageFaultHandler()");
    using namespace hal;
    ASSERT_NOT_NULL(data);

    PageFaultData pfd = ParsePageFaultData(*data);
    const auto &f_ptr = pfd.faulting_ptr;
    const auto &err   = pfd.error;

    TRACE_INFO_GENERAL(
        "PageFaultHandler: Addr=%p, P=%d, W=%d, U=%d, R=%d, I=%d", f_ptr, err.present, err.write,
        err.user, err.reserved_bits, err.instruction_fetch
    );

    auto &as           = MemoryModule::Get().GetKernelAddressSpace();
    auto area_or_error = as.FindArea(f_ptr);
    if (!area_or_error) {
        KernelPanicFormat("Page fault in unmapped memory at %p", f_ptr);
        return;
    }
    VPtr<VMemArea> vma_ptr = *area_or_error;
    const auto &vma        = *vma_ptr;

    if (!err.present) {  // Page just wasn't there
        if (err.write && !vma.flags.writable) {
            hal::KernelPanicFormat("Write to read-only memory area at %p", f_ptr);
            return;
        }

        auto &pmm = MemoryModule::Get().GetBitmapPmm();
        auto &mmu = MemoryModule::Get().GetMmu();

        PPtr<void> map_to = nullptr;
        if (vma.type == VirtualMemAreaT::Anonymous) {
            TRACE_INFO_GENERAL("PageFaultHandler: Handling Anonymous VMA");
            auto new_page_or_error = pmm.Alloc();
            if (!new_page_or_error) {
                hal::KernelPanic("Out of memory during page fault handling");
            }
            auto *new_page_phys = *new_page_or_error;
            auto *new_page_virt = Mem::PhysToVirt(new_page_phys);

            memset(new_page_virt, 0, hal::kPageSizeBytes);

            map_to = new_page_phys;
        } else if (vma.type == VirtualMemAreaT::DirectMapping) {
            TRACE_INFO_GENERAL("PageFaultHandler: Handling DirectMapping VMA");
            const auto offset = PtrToUptr(f_ptr) - PtrToUptr(vma.start);
            map_to            = UptrToPtr<void>(
                AlignDown(PtrToUptr(vma.direct_mapping_start) + offset, kPageSizeBytes)
            );
        } else {
            hal::KernelPanicFormat(
                "Unsupported VMA type %d at %p", static_cast<int>(vma.type), f_ptr
            );
            return;
        }

        hal::PageFlags page_flags{
            .Present        = true,
            .Writable       = vma.flags.writable,
            .UserAccessible = true,
            .WriteThrough   = vma.flags.write_through,
            .CacheDisable   = vma.flags.cache_disable,
            .Global         = false,
            .NoExecute      = !vma.flags.executable
        };

        // We need a context to Map.
        // For now, we construct a KernelMmuContext on the fly using the global PMM/PMT
        // FIXME: This is slightly inefficient to reconstruct on every fault, but valid.
        auto &ctx = MemoryModule::Get().GetKernelMmuContext();

        auto map_res = mmu.Map(
            ctx, as.PageTableRoot(), AlignDown(f_ptr, hal::kPageSizeBytes), map_to, page_flags
        );

        if (!map_res) {
            hal::KernelPanicFormat("Failed to map page for address %p", f_ptr);
        }

        TRACE_INFO_GENERAL(
            "Handled page fault at %p by mapping to physical page at %p", f_ptr, map_to
        );
        return;
    }

    hal::KernelPanicFormat("Unhandled page fault type at 0x%p", f_ptr);
}

}  // namespace Mem
