#include "modules/scheduling.hpp"

#include <cpu/control_registers.hpp>
#include <modules/memory.hpp>

#include "cpu/utils.hpp"
#include "hal/interrupt_params.hpp"
#include "hardware/core_local.hpp"
#include "mem/virt/addr_space.hpp"
#include "scheduling/thread.hpp"

// ------------------------------
// statics
// ------------------------------

FAST_CALL void SetThreadGs(Sched::Thread *thread)
{
    ASSERT_NOT_NULL(thread);

    cpu::SetMSR(arch::kIa32GsKernelBase, thread->gs_base);
}

FAST_CALL void DumpFpStateIfNeeded(Sched::Thread *thread)
{
    ASSERT_NOT_NULL(thread);

    auto proc = SchedulingModule::Get().GetProcesses().GetProcess(thread->owner);
    ASSERT_TRUE(static_cast<bool>(proc));

    if (proc.value()->flags.PreserveFloats) {
        u8 *mem = thread->fp_state;

        __asm__ volatile("xsave64 %0" : "=m"(*mem) : "a"(0xFFFFFFFF), "d"(0xFFFFFFFF) : "memory");
    }
}

FAST_CALL void LoadFpStateIfNeeded(Sched::Thread *thread)
{
    ASSERT_NOT_NULL(thread);

    auto proc = SchedulingModule::Get().GetProcesses().GetProcess(thread->owner);
    ASSERT_TRUE(static_cast<bool>(proc));

    if (proc.value()->flags.PreserveFloats) {
        u8 *mem = thread->fp_state;

        __asm__ volatile("xrstor64 %0" : : "m"(*mem), "a"(0xFFFFFFFF), "d"(0xFFFFFFFF) : "memory");
    }
}

FAST_CALL void SetTssRsp0(const u64 rsp0) { hardware::GetCoreLocalData().tss.rsp0 = rsp0; }

FAST_CALL void SetNextThreadFs(Sched::Thread *thread)
{
    ASSERT_NOT_NULL(thread);

    cpu::SetMSR(arch::kIa32FsBase, thread->fs_base);
}

FAST_CALL void SwapGsIfJumpingToUserspace(Sched::Thread *thread)
{
    ASSERT_NOT_NULL(thread);

    auto frame = static_cast<IsrErrorStackFrame *>(thread->kernel_stack);

    if (frame->isr_stack_frame.cs != cpu::GDT::kKernelCodeSelector) {
        SetThreadGs(thread);
        __asm__ volatile("swapgs" ::: "memory");
    }
}

NODISCARD FAST_CALL u64 GetRFlags()
{
    uint64_t flags;

    __asm__ volatile(
        "pushfq \n\t"
        "pop %0 \n\t"
        : "=r"(flags)
        :
        : "memory"
    );

    return flags;
}

FAST_CALL void SwapFsIfNeeded(Sched::Thread *current_tcb, Sched::Thread *next_tcb)
{
    ASSERT_NOT_NULL(current_tcb);
    ASSERT_NOT_NULL(next_tcb);

    if (current_tcb->fs_base != next_tcb->fs_base) {
        cpu::SetMSR(arch::kIa32FsBase, next_tcb->fs_base);
    }
}

FAST_CALL u64 GetThreadsPageTable(Sched::Thread *thread)
{
    ASSERT_NOT_NULL(thread);

    const auto proc = SchedulingModule::Get().GetProcesses().GetProcess(thread->owner);
    ASSERT_TRUE(static_cast<bool>(proc), "Threads exists -> owner MUST exist");

    return reinterpret_cast<u64>(proc.value()->address_space->PageTableRoot());
}

FAST_CALL void SwapAsIfNeeded(Sched::Thread *tcb)
{
    ASSERT_NOT_NULL(tcb);

    const auto proc = SchedulingModule::Get().GetProcesses().GetProcess(tcb->owner);
    ASSERT_TRUE(static_cast<bool>(proc), "Threads exists -> owner MUST exist");

    auto &current_as = MemoryModule::Get().GetVmm().GetCurrentAddressSpace();
    auto thread_as   = proc.value()->address_space;

    if (&current_as != thread_as) {
        MemoryModule::Get().GetVmm().SwitchAddrSpace(thread_as);
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

extern "C" void cdecl_JumpToUserSpaceEntry(void *addr, IsrStackFrame *frame)
{
    auto thread          = hardware::GetCurrentTCB();
    thread->kernel_stack = thread->kernel_stack_bottom;

    frame->rip    = reinterpret_cast<u64>(addr);
    frame->cs     = static_cast<u64>(cpu::GDT::kUserCodeSelector);
    frame->rflags = static_cast<u64>(kInitialRFlags);
    frame->rsp    = reinterpret_cast<u64>(thread->user_stack_bottom);
    frame->ss     = static_cast<u64>(cpu::GDT::kUserDataSelector);

    SetThreadGs(thread);
    __asm__ volatile("swapgs" ::: "memory");

    const auto proc = SchedulingModule::Get().GetProcesses().GetProcess(thread->owner);
    ASSERT_TRUE(static_cast<bool>(proc), "Threads exists -> owner MUST exist");
    MemoryModule::Get().GetVmm().SwitchAddrSpace(proc.value()->address_space);
}

extern "C" void cdecl_ContextSwitchEntry(
    Sched::Thread *thread, IsrErrorStackFrame *mem, const u64 rip
)
{
    mem->error_code             = 0;
    mem->isr_stack_frame.rip    = rip;
    mem->isr_stack_frame.cs     = static_cast<u64>(cpu::GDT::kKernelCodeSelector);
    mem->isr_stack_frame.rflags = GetRFlags();
    mem->isr_stack_frame.rsp    = reinterpret_cast<u64>(mem) + sizeof(IsrErrorStackFrame);
    mem->isr_stack_frame.ss     = static_cast<u64>(cpu::GDT::kKernelDataSelector);

    auto current_tcb = hardware::GetCurrentTCB();
    SwapFsIfNeeded(current_tcb, thread);
    current_tcb->kernel_stack = reinterpret_cast<void *>(mem);
    hardware::SetCurrentTCB(thread);
    SetTssRsp0(reinterpret_cast<u64>(thread->kernel_stack_bottom));
    SwapGsIfJumpingToUserspace(thread);
    LoadFpStateIfNeeded(thread);
    SwapAsIfNeeded(thread);
}

extern "C" void cdecl_ContextSwitchOnInterrupt(Sched::Thread *thread, void *rsp)
{
    auto current_tcb          = hardware::GetCurrentTCB();
    current_tcb->kernel_stack = rsp;
    SwapFsIfNeeded(current_tcb, thread);
    hardware::SetCurrentTCB(thread);
    SetTssRsp0(reinterpret_cast<u64>(thread->kernel_stack_bottom));
    SwapAsIfNeeded(thread);
}

// ===========================

extern "C" void cdecl_SetTssRsp0(const u64 rsp0) { SetTssRsp0(rsp0); }

extern "C" void cdecl_SwapFsIfNeeded(Sched::Thread *thread)
{
    SwapFsIfNeeded(hardware::GetCurrentTCB(), thread);
}

extern "C" void cdecl_SetNextThreadFs(Sched::Thread *thread) { SetNextThreadFs(thread); }

extern "C" void cdecl_DumpFpStateIfNeeded(Sched::Thread *thread) { DumpFpStateIfNeeded(thread); }

extern "C" void cdecl_LoadFpStateIfNeeded(Sched::Thread *thread) { LoadFpStateIfNeeded(thread); }

extern "C" Sched::Thread *cdecl_GetCurrentTCB() { return hardware::GetCurrentTCB(); }

extern "C" void cdecl_SetCurrentTCB(Sched::Thread *tcb) { hardware::SetCurrentTCB(tcb); }

extern "C" u64 cdecl_GetThreadsPageTable(Sched::Thread *thread)
{
    return GetThreadsPageTable(thread);
}
