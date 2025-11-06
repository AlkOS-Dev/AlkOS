#include <hal/impl/debug.hpp>
#include "trace_framework.hpp"

extern "C" {
extern char stack_top;
extern char stack_bottom;
}

namespace arch
{

void DebugStack()
{
    u64 rsp;
    asm volatile("mov %%rsp, %0" : "=r"(rsp));

    u64 top    = reinterpret_cast<u64>(&stack_top);
    u64 bottom = reinterpret_cast<u64>(&stack_bottom);

    u64 total = top - bottom;
    u64 used  = top - rsp;
    u64 left  = rsp - bottom;

    DEBUG_FREQ_INFO_GENERAL(
        "Stack [top=0x%016lx, "
        "bottom=0x%016lx, "
        "sp=0x%016lx, "
        "total=%luB, "
        "used=%luB, "
        "left=%luB]",
        top, bottom, rsp, total, used, left
    );
}

}  // namespace arch
