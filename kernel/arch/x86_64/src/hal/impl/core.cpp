#include <hal/impl/core.hpp>

#include "trace_framework.hpp"

using namespace arch;

void Core::EnableCore()
{
    // TODO:
}
void arch::InitializeCoreLocal()
{
    auto core_local = reinterpret_cast<CoreLocal *>(GetCoreLocalData());

    cpu::DefaultGdtInit(core_local->gdt, reinterpret_cast<u64>(&core_local->tss));
    core_local->gdtr.limit = sizeof(cpu::GDT) - 1;
    core_local->gdtr.base  = reinterpret_cast<u64>(&core_local->gdt);
}
