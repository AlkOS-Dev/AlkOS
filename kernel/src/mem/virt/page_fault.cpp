#include "mem/virt/page_fault.hpp"

#include <template/scope_guard.hpp>

#include "hal/intr_parser.hpp"
#include "hal/panic.hpp"
#include "mem/virt/addr_space.hpp"
#include "modules/memory.hpp"
#include "trace_framework.hpp"

namespace Mem
{

void PageFaultHandler(intr::LitExcEntry &, hal::ExceptionData *data)
{
    ASSERT_NOT_NULL(data);

    PageFaultData pfd = hal::ParsePageFaultData(*data);
    const auto &f_ptr = pfd.faulting_ptr;
    const auto &err   = pfd.error;

    auto &as = MemoryModule::Get().GetVmm().GetCurrentAddressSpace();

    as.Lock();
    template_lib::ScopeGuard guard([&]() {
        as.Unlock();
    });
    auto vma_res = as.FindAreaLocked(f_ptr);

    if (!vma_res) {
        hal::KernelPanicFormat("Page fault in unmapped memory at %p", f_ptr);
        return;
    }
    VMemArea *vma = *(*vma_res);

    if (err.present) {
        // Protection violation
        // TODO: CoW handling would go here via vma->HandleProtectionFault()
        hal::KernelPanicFormat("Unhandled protection fault at %p", f_ptr);
    }

    // Delegate to VMA
    // The VMA knows if it's anonymous, file-backed, or MMIO, and handles it accordingly.
    if (!vma->HandleFault(f_ptr, err, as)) {
        hal::KernelPanicFormat("Failed to resolve page fault at %p", f_ptr);
    }
}

}  // namespace Mem
