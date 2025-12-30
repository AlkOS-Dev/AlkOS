#include "hal/impl/scheduling.hpp"
#include "cpu/gdt.hpp"
#include "cpu/utils.hpp"
#include "hal/interrupt_params.hpp"
#include "scheduling/threads.hpp"

#include <string.h>
#include <bits_ext.hpp>

template <bool kIsKernelTask = true>
FAST_CALL void InitializeStack(void **stack, void (*f)())
{
    /* NOTE: Thread entry always starts in Kernel Code */
    static constexpr auto kFunc =
        kIsKernelTask ? Sched::KThreadEntrypoint : Sched::UserThreadEntrypoint;

    auto stack_top = static_cast<byte *>(*stack) - sizeof(IsrErrorStackFrame);
    auto frame     = reinterpret_cast<IsrErrorStackFrame *>(stack_top);

    memset(stack_top, 0, sizeof(IsrErrorStackFrame));

    frame->registers.rdi          = reinterpret_cast<u64>(f);
    frame->isr_stack_frame.rip    = reinterpret_cast<u64>(kFunc);
    frame->error_code             = 0;
    frame->isr_stack_frame.cs     = static_cast<u64>(cpu::GDT::kKernelCodeSelector);
    frame->isr_stack_frame.rflags = kInitialRFlags;
    frame->isr_stack_frame.rsp    = reinterpret_cast<u64>(*stack);
    frame->isr_stack_frame.ss     = static_cast<u64>(cpu::GDT::kKernelDataSelector);

    *stack = reinterpret_cast<void *>(stack_top);
}

namespace arch
{
void InitializeStackKThread(void **stack, void (*f)()) { InitializeStack<true>(stack, f); }

void InitializeStackUserThread(void **stack, void (*f)()) { InitializeStack<false>(stack, f); }

}  // namespace arch
