#include "hal/impl/scheduling.hpp"
#include "scheduling/threads.hpp"

#include <string.h>

template <auto func = Sched::KThreadEntrypoint>
FAST_CALL void InitializeStack(void **stack, void (*f)())
{
    static constexpr size_t kStackSpace = 16 * 8;  // 15 regs + rip
    static constexpr size_t kRipOffset  = 15 * 8;  // 15 regs
    static constexpr size_t kRdiOffset  = 4 * 8;   // rdi = 5th reg

    auto stack_top = static_cast<byte *>(*stack) - kStackSpace;

    memset(stack_top, 0, kStackSpace);
    *reinterpret_cast<u64 *>(stack_top + kRdiOffset) = reinterpret_cast<u64>(f);
    *reinterpret_cast<u64 *>(stack_top + kRipOffset) = reinterpret_cast<u64>(func);

    *stack = reinterpret_cast<void *>(stack_top);
}

namespace arch
{
void InitializeStackKThread(void **stack, void (*f)())
{
    InitializeStack<Sched::KThreadEntrypoint>(stack, f);
}

void InitializeStackUserThread(void **stack, void (*f)())
{
    InitializeStack<Sched::UserThreadEntrypoint>(stack, f);
}

}  // namespace arch
