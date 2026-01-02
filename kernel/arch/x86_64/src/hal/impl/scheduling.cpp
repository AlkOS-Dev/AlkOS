#include "hal/impl/scheduling.hpp"
#include "cpu/gdt.hpp"
#include "cpu/utils.hpp"
#include "hal/interrupt_params.hpp"
#include "scheduling/threads.hpp"

#include <string.h>
#include <bits_ext.hpp>

namespace arch
{
void InitializeThreadStack(void **stack, const Sched::Task &task)
{
    /* NOTE: Thread entry always starts in Kernel Code */
    auto stack_top = static_cast<byte *>(*stack) - sizeof(IsrErrorStackFrame);
    auto frame     = reinterpret_cast<IsrErrorStackFrame *>(stack_top);

    memset(stack_top, 0, sizeof(IsrErrorStackFrame));

    /* Initialize IsrErrorStackFrame */
    frame->isr_stack_frame.rip    = reinterpret_cast<u64>(task.func);
    frame->error_code             = 0;
    frame->isr_stack_frame.cs     = static_cast<u64>(cpu::GDT::kKernelCodeSelector);
    frame->isr_stack_frame.rflags = kInitialRFlags;
    frame->isr_stack_frame.rsp    = reinterpret_cast<u64>(*stack);
    frame->isr_stack_frame.ss     = static_cast<u64>(cpu::GDT::kKernelDataSelector);

    /* Initialize function arguments */
    if (task.args_count > 0) {
        frame->registers.rdi = task.args[0];
    }

    if (task.args_count > 1) {
        frame->registers.rsi = task.args[1];
    }

    if (task.args_count > 2) {
        frame->registers.rdx = task.args[2];
    }

    if (task.args_count > 3) {
        frame->registers.rcx = task.args[3];
    }

    if (task.args_count > 4) {
        frame->registers.r8 = task.args[4];
    }

    if (task.args_count > 5) {
        frame->registers.r9 = task.args[5];
    }

    /* Save adjusted stack address */
    *stack = reinterpret_cast<void *>(stack_top);
}
}  // namespace arch
