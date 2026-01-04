#include "modules/scheduling.hpp"
#include "cpu/control_registers.hpp"
#include "cpu/utils.hpp"
#include "hal/interrupt_params.hpp"
#include "hardware/core_local.hpp"
#include "mem/virt/addr_space.hpp"
#include "modules/memory.hpp"
#include "modules/timing.hpp"
#include "scheduling/thread.hpp"

// ------------------------------
// statics
// ------------------------------

FAST_CALL void SetThreadGs(Sched::Thread *thread)
{
    ASSERT_NOT_NULL(thread);

    cpu::SetMSR(arch::kIa32GsKernelBase, thread->arch_data.gs_base);
}

FAST_CALL void DumpFpStateIfNeeded(Sched::Thread *thread)
{
    ASSERT_NOT_NULL(thread);

    if (thread->flags.preserve_floats) {
        u8 *mem = thread->arch_data.fp_state;

        __asm__ volatile("xsave64 %0" : "=m"(*mem) : "a"(0xFFFFFFFF), "d"(0xFFFFFFFF) : "memory");
    }
}

FAST_CALL void LoadFpStateIfNeeded(Sched::Thread *thread)
{
    ASSERT_NOT_NULL(thread);

    if (thread->flags.preserve_floats) {
        u8 *mem = thread->arch_data.fp_state;

        __asm__ volatile("xrstor64 %0" : : "m"(*mem), "a"(0xFFFFFFFF), "d"(0xFFFFFFFF) : "memory");
    }
}

FAST_CALL void SetTssRsp0(const u64 rsp0) { hardware::GetCoreLocalSelf()->tss.rsp0 = rsp0; }

FAST_CALL void SetNextThreadFs(Sched::Thread *thread)
{
    ASSERT_NOT_NULL(thread);

    cpu::SetMSR(arch::kIa32FsBase, thread->arch_data.fs_base);
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

    if (current_tcb->arch_data.fs_base != next_tcb->arch_data.fs_base) {
        cpu::SetMSR(arch::kIa32FsBase, next_tcb->arch_data.fs_base);
    }
}

FAST_CALL void SwapAsIfNeeded(Sched::Thread *tcb)
{
    ASSERT_NOT_NULL(tcb);

    const auto proc = SchedulingModule::Get().GetProcesses().GetProcess(tcb->owner);
    ASSERT_TRUE(static_cast<bool>(proc), "Threads exists -> owner MUST exist");

    const auto current_as = &MemoryModule::Get().GetVmm().GetCurrentAddressSpace();
    const auto thread_as  = proc.value()->address_space;

    if (current_as != thread_as) {
        MemoryModule::Get().GetVmm().SwitchAddrSpace(thread_as);
    }
}

// ------------------------------
// asm interfaces
// ------------------------------

extern "C" void cdecl_ConvertContextEntry(Sched::Thread *thread)
{
    ASSERT_NOT_NULL(thread);
    ASSERT_EQ(thread->state, Sched::ThreadState::kRunning);

    SetNextThreadFs(thread);
    LoadFpStateIfNeeded(thread);
    hardware::SetCoreLocalTcb(thread);
    SetTssRsp0(reinterpret_cast<u64>(thread->kernel_stack_bottom));
    thread->timestamp = TimingModule::Get().GetSystemTime().ReadLifeTimeNs();
    SwapAsIfNeeded(thread);
    SwapGsIfJumpingToUserspace(thread);
}

extern "C" void cdecl_JumpToUserSpaceEntry(void *addr, IsrStackFrame *frame)
{
    auto thread          = hardware::GetCoreLocalTcb();
    thread->kernel_stack = thread->kernel_stack_bottom;

    frame->rip    = reinterpret_cast<u64>(addr);
    frame->cs     = static_cast<u64>(cpu::GDT::kUserCodeSelector);
    frame->rflags = static_cast<u64>(kInitialRFlags);
    frame->rsp    = reinterpret_cast<u64>(thread->user_stack_bottom);
    frame->ss     = static_cast<u64>(cpu::GDT::kUserDataSelector);

    const u64 t            = TimingModule::Get().GetSystemTime().ReadLifeTimeNs();
    thread->kernel_time_ns = t - thread->timestamp;
    thread->timestamp      = t;

    SetThreadGs(thread);
    __asm__ volatile("swapgs" ::: "memory");
}

extern "C" void cdecl_ContextSwitchEntry(
    Sched::Thread *thread, IsrErrorStackFrame *mem, const u64 rip
)
{
    ASSERT_EQ(thread->state, Sched::ThreadState::kRunning);
    ASSERT_EQ(hardware::GetCoreLocalTcb()->state, Sched::ThreadState::kReady);

    const auto current_tcb = hardware::GetCoreLocalTcb();
    DumpFpStateIfNeeded(current_tcb);

    mem->error_code             = 0;
    mem->isr_stack_frame.rip    = rip;
    mem->isr_stack_frame.cs     = static_cast<u64>(cpu::GDT::kKernelCodeSelector);
    mem->isr_stack_frame.rflags = GetRFlags();
    mem->isr_stack_frame.rsp    = reinterpret_cast<u64>(mem) + sizeof(IsrErrorStackFrame);
    mem->isr_stack_frame.ss     = static_cast<u64>(cpu::GDT::kKernelDataSelector);

    SwapFsIfNeeded(current_tcb, thread);
    current_tcb->kernel_stack = reinterpret_cast<void *>(mem);
    hardware::SetCoreLocalTcb(thread);
    SetTssRsp0(reinterpret_cast<u64>(thread->kernel_stack_bottom));

    const u64 t                 = TimingModule::Get().GetSystemTime().ReadLifeTimeNs();
    current_tcb->kernel_time_ns = t - current_tcb->timestamp;
    current_tcb->num_context_switches++;
    thread->timestamp = t;

    LoadFpStateIfNeeded(thread);
    SwapAsIfNeeded(thread);
    SwapGsIfJumpingToUserspace(thread);
}

extern "C" void cdecl_ContextSwitchOnInterrupt(Sched::Thread *thread, void *rsp)
{
    ASSERT_EQ(thread->state, Sched::ThreadState::kRunning);
    ASSERT_EQ(hardware::GetCoreLocalTcb()->state, Sched::ThreadState::kReady);

    const auto current_tcb = hardware::GetCoreLocalTcb();
    DumpFpStateIfNeeded(current_tcb);

    current_tcb->kernel_stack = rsp;
    SwapFsIfNeeded(current_tcb, thread);
    hardware::SetCoreLocalTcb(thread);
    SetTssRsp0(reinterpret_cast<u64>(thread->kernel_stack_bottom));

    LoadFpStateIfNeeded(thread);
    SwapAsIfNeeded(thread);
}
