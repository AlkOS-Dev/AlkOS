#include "interrupts/interrupt_types.hpp"

#include "hal/intr_parser.hpp"
#include "hal/panic.hpp"
#include "mem/virt/page_fault.hpp"
#include "modules/hardware.hpp"
#include "modules/memory.hpp"

namespace Mem
{

void PageFaultHandler(intr::LitExcEntry &, hal::ExceptionData *data)
{
    using namespace hal;
    ASSERT_NOT_NULL(data);

    PageFaultData pfd = ParsePageFaultData(*data);
    const auto &f_ptr = pfd.faulting_ptr;
    const auto &err   = pfd.error;

    auto &as           = MemoryModule::Get().GetKernelAddressSpace();
    auto area_or_error = as.FindArea(f_ptr);
    if (!area_or_error) {
        KernelPanicFormat("Page fault in unmapped memory at 0x%p", f_ptr);
        return;
    }
    VPtr<VMemArea> vma_ptr = *area_or_error;
    const auto &vma        = *vma_ptr;

    if (!err.present) {  // Page just wasn't there
        if (err.write && !vma.flags.writable) {
            hal::KernelPanicFormat("Write to read-only memory area at 0x%p", f_ptr);
            return;
        }

        auto &pmm = MemoryModule::Get().GetBitmapPmm();
        auto &mmu = MemoryModule::Get().GetMmu();

        PPtr<void> map_to = nullptr;
        if (vma.type == VirtualMemAreaT::Anonymous) {
            auto new_page_or_error = pmm.Alloc();
            if (!new_page_or_error) {
                hal::KernelPanic("Out of memory during page fault handling");
            }
            auto new_page_phys = *new_page_or_error;
            auto new_page_virt = Mem::PhysToVirt(new_page_phys);

            memset(new_page_virt, 0, hal::kPageSizeBytes);

            map_to = new_page_phys;
        } else if (vma.type == VirtualMemAreaT::DirectMapping) {
            const auto offset = PtrToUptr(f_ptr) - PtrToUptr(vma.start);
            map_to            = UptrToPtr<void>(
                AlignDown(PtrToUptr(vma.direct_mapping_start) + offset, kPageSizeBytes)
            );
        } else {
            hal::KernelPanicFormat(
                "Unsupported VMA type %d at 0x%p", static_cast<int>(vma.type), f_ptr
            );
            return;
        }

        hal::PageFlags page_flags{
            .Present        = true,
            .Writable       = vma.flags.writable,
            .UserAccessible = false,
            .WriteThrough   = false,
            .CacheDisable   = false,
            .Global         = false,
            .NoExecute      = !vma.flags.executable
        };

        auto map_res = mmu.Map(&as, AlignDown(f_ptr, hal::kPageSizeBytes), map_to, page_flags);

        if (!map_res) {
            hal::KernelPanicFormat("Failed to map page for address 0x%p", f_ptr);
        }

        TRACE_SUCCESS(
            "Handled page fault at 0x%p by mapping to physical page at 0x%p", f_ptr, map_to
        );
        return;
    }

    hal::KernelPanicFormat("Unhandled page fault type at 0x%p", f_ptr);
}

}  // namespace Mem
