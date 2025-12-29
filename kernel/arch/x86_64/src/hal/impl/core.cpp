#include <hal/core.hpp>

#include "cpu/gdt.hpp"
#include "hardware/core_local.hpp"
#include "trace_framework.hpp"

extern "C" void GdtFlush(cpu::Gdtr *gdtr, u64 kernel_code_offset, u64 kernel_data_offset);

namespace arch
{
void Core::EnableCore()
{
    // TODO:
}

void InitializeCoreLocal()
{
    auto *core_local = static_cast<hardware::CoreLocal *>(GetCoreLocalData());

    cpu::DefaultGdtInit(core_local->gdt, reinterpret_cast<u64>(&core_local->tss));
    core_local->gdtr.limit = sizeof(cpu::GDT) - 1;
    core_local->gdtr.base  = reinterpret_cast<u64>(&core_local->gdt);

    GdtFlush(&core_local->gdtr, cpu::GDT::kKernelCodeSelector, cpu::GDT::kKernelDataSelector);
    SetCoreLocalData(core_local);
    cpu::LoadTss(cpu::GDT::kTssSelector);

    DEBUG_INFO_HARDWARE(
        "Successfully initialized GDT and TSS for core with id %hu", core_local->lid
    );
}
}  // namespace arch

void cdecl_SetTssRsp0(const u64 rsp0) { arch::SetTssRsp0(rsp0); }

void cdecl_SwapFsIfNeeded(Sched::Thread *thread)
{
    auto tcb = hardware::GetCurrentTCB();

    if (tcb->fs_base != thread->fs_base) {
        cpu::SetMSR(arch::kIa32FsBase, thread->fs_base);
    }
}

void cdecl_SetNextThreadFs(Sched::Thread *thread)
{
    cpu::SetMSR(arch::kIa32FsBase, thread->fs_base);
}

void cdecl_LoadThreadsGs(Sched::Thread *thread) { cpu::SetMSR(arch::kIa32GsBase, thread->gs_base); }
