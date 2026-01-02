#include "mem/virt/page_fault.hpp"

#include <template/scope_guard.hpp>

#include "hal/intr_parser.hpp"
#include "hal/panic.hpp"
#include "mem/virt/addr_space.hpp"
#include "modules/memory.hpp"
#include "trace_framework.hpp"

namespace Mem
{

namespace
{
constexpr const char *PresentDescription(bool present)
{
    return present ? "Page-protection violation" : "Page was not present";
}

constexpr const char *WriteDescription(bool write)
{
    return write ? "Write operation" : "Read operation";
}

constexpr const char *UserModeDescription(bool user)
{
    return user ? "Occurred in user mode" : "Occurred in kernel mode";
}

constexpr const char *ReservedBitsDescription(bool reserved_bits)
{
    return reserved_bits ? "One or more reserved bits were set in a page-table entry"
                         : "No reserved bits issue";
}

constexpr const char *InstructionFetchDescription(bool instruction_fetch)
{
    return instruction_fetch ? "Fault caused by an instruction fetch"
                             : "Fault not caused by an instruction fetch";
}
}  // namespace

void *PageFaultHandler(intr::LitExcEntry &, hal::ExceptionData *data)
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
        hal::KernelPanicFormat(
            "Page fault in unmapped memory\n"
            "Faulting address: %p\n"
            "Instruction pointer: %p\n"
            "Error details:\n"
            "  - %s\n"
            "  - %s\n"
            "  - %s\n"
            "  - %s\n"
            "  - %s",
            f_ptr, data->isr_stack_frame.rip, PresentDescription(err.present),
            WriteDescription(err.write), UserModeDescription(err.user),
            ReservedBitsDescription(err.reserved_bits),
            InstructionFetchDescription(err.instruction_fetch)
        );
        return nullptr;
    }
    VMemArea *vma = *(*vma_res);

    if (err.present) {
        // Protection violation
        // TODO: CoW handling would go here via vma->HandleProtectionFault()
        hal::KernelPanicFormat(
            "Unhandled protection fault\n"
            "Faulting address: %p\n"
            "Instruction pointer: %p\n"
            "Error details:\n"
            "  - %s\n"
            "  - %s\n"
            "  - %s\n"
            "  - %s\n"
            "  - %s",
            f_ptr, data->isr_stack_frame.rip, PresentDescription(err.present),
            WriteDescription(err.write), UserModeDescription(err.user),
            ReservedBitsDescription(err.reserved_bits),
            InstructionFetchDescription(err.instruction_fetch)
        );
    }

    // Delegate to VMA
    // The VMA knows if it's anonymous, file-backed, or MMIO, and handles it accordingly.
    if (!vma->HandleFault(f_ptr, err, as)) {
        hal::KernelPanicFormat(
            "Failed to resolve page fault\n"
            "Faulting address: %p\n"
            "Instruction pointer: %p\n"
            "Error details:\n"
            "  - %s\n"
            "  - %s\n"
            "  - %s\n"
            "  - %s\n"
            "  - %s",
            f_ptr, data->isr_stack_frame.rip, PresentDescription(err.present),
            WriteDescription(err.write), UserModeDescription(err.user),
            ReservedBitsDescription(err.reserved_bits),
            InstructionFetchDescription(err.instruction_fetch)
        );
    }
    return nullptr;
}

}  // namespace Mem
