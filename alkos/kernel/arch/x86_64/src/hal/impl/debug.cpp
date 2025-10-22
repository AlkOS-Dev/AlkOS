#include <extensions/debug.hpp>
#include <hal/impl/debug.hpp>

extern u64 stack_top;
extern u64 stack_bottom;
namespace arch
{
void DebugStack()
{
    u64 rsp;
    asm volatile("mov %%rsp, %0" : "=r"(rsp));

    u64 total = stack_top - stack_bottom;
    u64 used  = stack_top - rsp;
    u64 left  = rsp - stack_bottom;

    TRACE_DEBUG(
        "Stack [top=0x%016lx, "
        "bottom=0x%016lx, "
        "sp=0x%016lx, "
        "total=%luB, "
        "used=%luB, "
        "left=%luB]",
        stack_top, stack_bottom, rsp, total, used, left
    );
}
}  // namespace arch
