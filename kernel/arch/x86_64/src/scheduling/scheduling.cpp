#include "modules/scheduling.hpp"
#include "cpu/utils.hpp"
#include "hal/interrupt_params.hpp"
#include "hardware/core_local.hpp"
#include "scheduling/thread.hpp"

// ------------------------------
// statics
// ------------------------------

FAST_CALL void SetThreadGs(Sched::Thread *thread)
{
    cpu::SetMSR(arch::kIa32GsKernelBase, thread->gs_base);
}

FAST_CALL void DumpFpStateIfNeeded(Sched::Thread *thread)
{
    auto proc = SchedulingModule::Get().GetProcesses().GetProcess(thread->owner);
    ASSERT_TRUE(static_cast<bool>(proc));

    if (proc.value()->flags.PreserveFloats) {
        u8 *mem = thread->fp_state;

        __asm__ volatile("xsave64 %0" : "=m"(*mem) : "a"(0xFFFFFFFF), "d"(0xFFFFFFFF) : "memory");
    }
}

FAST_CALL void LoadFpStateIfNeeded(Sched::Thread *thread)
{
    auto proc = SchedulingModule::Get().GetProcesses().GetProcess(thread->owner);
    ASSERT_TRUE(static_cast<bool>(proc));

    if (proc.value()->flags.PreserveFloats) {
        u8 *mem = thread->fp_state;

        __asm__ volatile("xrstor64 %0" : : "m"(*mem), "a"(0xFFFFFFFF), "d"(0xFFFFFFFF) : "memory");
    }
}

FAST_CALL void SetTssRsp0(const u64 rsp0)
{
    auto core_local     = hardware::GetCoreLocalData();
    core_local.tss.rsp0 = rsp0;
}

FAST_CALL void SetNextThreadFs(Sched::Thread *thread)
{
    cpu::SetMSR(arch::kIa32FsBase, thread->fs_base);
}

FAST_CALL void SwapGsIfJumpingToUserspace(Sched::Thread *thread)
{
    auto frame = static_cast<IsrErrorStackFrame *>(thread->kernel_stack);

    if (frame->isr_stack_frame.cs != cpu::GDT::kKernelCodeSelector) {
        SetThreadGs(thread);
        __asm__ volatile("swapgs" ::: "memory");
    }
}

// ------------------------------
// asm interfaces
// ------------------------------

extern "C" void cdecl_ConvertContextEntry(Sched::Thread *thread)
{
    SetNextThreadFs(thread);
    LoadFpStateIfNeeded(thread);
    hardware::SetCurrentTCB(thread);
    SetTssRsp0(reinterpret_cast<u64>(thread->kernel_stack_bottom));
    SwapGsIfJumpingToUserspace(thread);
}

extern "C" void cdecl_JumpToUserSpaceEntry(void *addr, void *mem)
{
    auto frame = static_cast<IsrStackFrame *>(mem);

    auto thread          = hardware::GetCurrentTCB();
    thread->kernel_stack = thread->kernel_stack_bottom;

    frame->rip    = reinterpret_cast<u64>(addr);
    frame->cs     = static_cast<u64>(cpu::GDT::kUserCodeSelector);
    frame->rflags = static_cast<u64>(kInitialRFlags);
    frame->rsp    = reinterpret_cast<u64>(thread->user_stack_bottom);
    frame->ss     = static_cast<u64>(cpu::GDT::kUserDataSelector);

    SetThreadGs(thread);
    __asm__ volatile("swapgs" ::: "memory");
}

// ===========================

extern "C" void cdecl_SetTssRsp0(const u64 rsp0) { SetTssRsp0(rsp0); }

extern "C" void cdecl_SwapFsIfNeeded(Sched::Thread *thread)
{
    auto tcb = hardware::GetCurrentTCB();

    if (tcb->fs_base != thread->fs_base) {
        cpu::SetMSR(arch::kIa32FsBase, thread->fs_base);
    }
}

extern "C" void cdecl_SetNextThreadFs(Sched::Thread *thread) { SetNextThreadFs(thread); }

extern "C" void cdecl_DumpFpStateIfNeeded(Sched::Thread *thread) { DumpFpStateIfNeeded(thread); }

extern "C" void cdecl_LoadFpStateIfNeeded(Sched::Thread *thread) { LoadFpStateIfNeeded(thread); }

extern "C" Sched::Thread *cdecl_GetCurrentTCB() { return hardware::GetCurrentTCB(); }

extern "C" void cdecl_SetCurrentTCB(Sched::Thread *tcb) { hardware::SetCurrentTCB(tcb); }
