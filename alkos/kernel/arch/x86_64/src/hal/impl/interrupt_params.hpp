#ifndef ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_INTERRUPT_PARAMS_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_INTERRUPT_PARAMS_HPP_

#include <defines.h>

struct PACK IsrStackFrame {
    u64 rip;
    u64 cs;
    u64 rflags;
    u64 rsp;
};

struct PACK IsrErrorStackFrame {
    u64 error_code;
    IsrStackFrame isr_stack_frame;
};

namespace arch
{
static constexpr size_t kNumExceptionHandlers   = 32;
static constexpr size_t kNumHardwareInterrupts  = 32;
static constexpr size_t kNumSoftwareInterrupts  = 32;
static constexpr size_t kNumX86_64CpuExceptions = 32;
static constexpr size_t kNumX86_64Irqs          = 16;

using ExceptionData = IsrErrorStackFrame;
}  // namespace arch

#endif  // ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_INTERRUPT_PARAMS_HPP_
