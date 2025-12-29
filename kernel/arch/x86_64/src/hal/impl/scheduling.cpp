#include "hal/impl/scheduling.hpp"
#include "cpu/gdt.hpp"
#include "scheduling/threads.hpp"

#include <string.h>
#include <bits_ext.hpp>

template <bool kIsKernelTask = true>
FAST_CALL void InitializeStack(void **stack, void (*f)())
{
    /* NOTE: Thread entry always starts in Kernel Code */
    static constexpr auto kFunc =
        kIsKernelTask ? Sched::KThreadEntrypoint : Sched::UserThreadEntrypoint;
    static constexpr u64 kInitialFlags =
        kSingleBit<u64, 1> | kSingleBit<u64, 9>;  // Bit1 - Reserved, Bit9 - Interrupt flag

    static constexpr size_t kStackSpace  = 21 * 8;  // 15 regs + InterruptStackFrame
    static constexpr size_t kRdiOffset   = 4 * 8;   // rdi = 5th reg
    static constexpr size_t kRipOffset   = 16 * 8;  // 15 regs + error code
    static constexpr size_t kCsOffset    = kRipOffset + sizeof(u64);
    static constexpr size_t kFlagsOffset = kCsOffset + sizeof(u64);
    static constexpr size_t kSpOffset    = kFlagsOffset + sizeof(u64);
    static constexpr size_t kSsOffset    = kSpOffset + sizeof(u64);

    auto stack_top = static_cast<byte *>(*stack) - kStackSpace;

    memset(stack_top, 0, kStackSpace);
    *reinterpret_cast<u64 *>(stack_top + kRdiOffset) = reinterpret_cast<u64>(f);
    *reinterpret_cast<u64 *>(stack_top + kRipOffset) = reinterpret_cast<u64>(kFunc);
    *reinterpret_cast<u64 *>(stack_top + kCsOffset) =
        static_cast<u64>(cpu::GDT::kKernelCodeSelector);
    *reinterpret_cast<u64 *>(stack_top + kSsOffset) =
        static_cast<u64>(cpu::GDT::kKernelDataSelector);
    *reinterpret_cast<u64 *>(stack_top + kSpOffset)    = reinterpret_cast<u64>(*stack);
    *reinterpret_cast<u64 *>(stack_top + kFlagsOffset) = kInitialFlags;

    *stack = reinterpret_cast<void *>(stack_top);
}

namespace arch
{
void InitializeStackKThread(void **stack, void (*f)()) { InitializeStack<true>(stack, f); }

void InitializeStackUserThread(void **stack, void (*f)()) { InitializeStack<false>(stack, f); }

}  // namespace arch
