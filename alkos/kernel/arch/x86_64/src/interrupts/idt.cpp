#include "interrupts/idt.hpp"
#include <extensions/defines.hpp>

#include <hal/debug.hpp>
#include <hal/panic.hpp>  // I dislike this import architecturally, but let it be for now
#include <modules/timing.hpp>
#include <trace.hpp>

#include "cpu/utils.hpp"
#include "hal/interrupts.hpp"
#include "interrupts/interrupt_types.hpp"

#include <assert.h>
#include <extensions/bit.hpp>
#include <extensions/debug.hpp>

static constexpr u32 kStubTableSize = 64;

/**
 * Flags for ISRs (Interrupt Service Routines):
 * - 0xE: 64-bit Interrupt Gate (0xF for 64-bit Trap Gate)
 * - Bit 4: Reserved (must always be 0)
 * - Bits 5-6: 2-bit value defining the allowed CPU Privilege Levels (CPL)
 *   that can access this interrupt via the INT instruction. Hardware interrupts ignore this.
 * - Bit 7: Present bit (must be set to 1 for the descriptor to be valid).
 */
static constexpr u8 kTrapFlags      = 0x8F;
static constexpr u8 kInterruptFlags = 0x8E;

/* gdt kernel code offset */
extern "C" u32 kKernelCodeOffset;

/* isr stub table initialized in nasm */
extern "C" void *IsrWrapperTable[];

// ------------------------------
// Isrs
// ------------------------------

/**
 * @brief This handler is called when an unknown interrupt occurs.
 * Executes KernelPanic.
 *
 * @param idt_idx index of interrupt triggered.
 */
NO_RET void DefaultInterruptHandler(const u8 idt_idx)
{
    hal::KernelPanicFormat("Received unsupported interrupt with code: %hhu\n", idt_idx);
}

NO_RET FAST_CALL void DefaultExceptionHandler(IsrErrorStackFrame *stack_frame, const u8 idt_idx)
{
    static constexpr size_t kStateMsgSize = 1024;

    CpuState cpu_state = DumpCpuState();

    /* restore relevant registers to the state before printing msg */
    cpu_state.general_purpose_registers[CpuState::kRdi] =
        *(reinterpret_cast<u64 *>(stack_frame) - 1);
    cpu_state.general_purpose_registers[CpuState::kRsi] =
        *(reinterpret_cast<u64 *>(stack_frame) - 2);
    cpu_state.general_purpose_registers[CpuState::kRsp] = stack_frame->isr_stack_frame.rsp;

    char state_buffer[kStateMsgSize];
    cpu_state.GetStateDesc(state_buffer, kStateMsgSize);

    TRACE_DEBUG("what: %hhu", idt_idx);
    const char *exception_msg = GetExceptionMsg(idt_idx);
    R_ASSERT_NOT_NULL(exception_msg);

    hal::KernelPanicFormat(
        "Received exception: %d (%s)\n"
        "And error: %llu\n"
        "At instruction address: 0x%016llx\n"
        "%s\n"
        "RFLAGS: 0x%016llx\n",
        idt_idx, exception_msg, stack_frame->error_code, stack_frame->isr_stack_frame.rip,
        state_buffer, stack_frame->isr_stack_frame.rflags
    );
}

void DefaultExceptionHandler(intr::LitExcEntry &entry, hal::ExceptionData *data)
{
    DefaultExceptionHandler(data, entry.hardware_irq);
}

FAST_CALL void LogIrqReceived(const u16 idt_idx, const u16 lirq)
{
    KernelTraceInfo("Received interrupt with idt idx: %hu and lirq: %hu", idt_idx, lirq);
}

void SimpleIrqHandler(intr::LitHwEntry &entry)
{
    LogIrqReceived(entry.hardware_irq, entry.logical_irq);
}

void TestIsr(intr::LitSwEntry &entry)
{
    LogIrqReceived(entry.hardware_irq, entry.logical_irq);

    /* pollute all registers possible */
    __asm__ volatile("movq $-1, %%rax" : : : "rax");
    __asm__ volatile("movq $-1, %%rbx" : : : "rbx");
    __asm__ volatile("movq $-1, %%rcx" : : : "rcx");
    __asm__ volatile("movq $-1, %%rdx" : : : "rdx");
    __asm__ volatile("movq $-1, %%rsi" : : : "rsi");
    __asm__ volatile("movq $-1, %%rdi" : : : "rdi");
    __asm__ volatile("movq $-1, %%r8" : : : "r8");
    __asm__ volatile("movq $-1, %%r9" : : : "r9");
    __asm__ volatile("movq $-1, %%r10" : : : "r10");
    __asm__ volatile("movq $-1, %%r11" : : : "r11");
    __asm__ volatile("movq $-1, %%r12" : : : "r12");
    __asm__ volatile("movq $-1, %%r13" : : : "r13");
    __asm__ volatile("movq $-1, %%r14" : : : "r14");
    __asm__ volatile("movq $-1, %%r15" : : : "r15");
}

void TimerIsr(intr::LitHwEntry &entry)
{
    (void)entry;  // Unused parameter
    // LogIrqReceived(entry.hardware_irq, entry.logical_irq);

    hal::DebugStack();

    // TODO: Temporary code
    static u64 counter = 0;
    if (!FeatureEnabled<FeatureFlag::kRunTestMode> && counter++ % 33 == 0) {
        static constexpr size_t kBuffSize = 256;
        char buff[kBuffSize]{};

        if (TimingModule::IsInited()) {
            const auto t = time(nullptr);
            strftime(buff, kBuffSize, "%Y-%m-%d %H:%M:%S", localtime(&t));
        }

        KernelTraceInfo(
            "Kernel time update: %s, have a nice day!",
            TimingModule::IsInited() ? buff : "Timing module is not initialized!"
        );
    }
}

// ------------------------------
// Functions
// ------------------------------

/**
 * @brief Checks if the interrupt is a trap entry.
 * @param idx - index of the interrupt
 * @return whether the interrupt is a trap entry
 */
static bool IsTrapEntry(const u8 idx)
{
    for (const u8 trap_idx : kExceptionIdx) {
        if (idx == trap_idx) {
            return true;
        }
    }

    return false;
}

/**
 * @note returns nullptr if the exception index is not found
 */
const char *GetExceptionMsg(const u8 exc_idx)
{
    for (size_t idx = 0; idx < kExceptionCount; ++idx) {
        if (kExceptionIdx[idx] == exc_idx) {
            return kExceptionMsg[idx];
        }
    }

    return nullptr;
}

static void IdtSetDescriptor(Idt &idt, const u8 idx, const u64 isr, const u8 flags)
{
    IdtEntry &entry = idt.idt[idx];

    entry.isr_low    = isr & kBitMask16;
    entry.kernel_cs  = kKernelCodeOffset;
    entry.ist        = 0;
    entry.attributes = flags;
    entry.isr_mid    = (isr >> 16) & kBitMask16;
    entry.isr_high   = (isr >> 32) & kBitMask32;
    entry.reserved   = 0;
}

void arch::Interrupts::InitializeDefaultIdt_()
{
    R_ASSERT_LT(kKernelCodeOffset, static_cast<u32>(UINT16_MAX));
    R_ASSERT_NEQ(static_cast<u32>(0), kKernelCodeOffset);

    idt_.idtr.base  = reinterpret_cast<uintptr_t>(idt_.idt);
    idt_.idtr.limit = static_cast<u16>(sizeof(IdtEntry)) * kIdtEntries - 1;

    for (u8 idx = 0; idx < kStubTableSize; ++idx) {
        IdtSetDescriptor(
            idt_, idx, reinterpret_cast<u64>(IsrWrapperTable[idx]),
            IsTrapEntry(idx) ? kTrapFlags : kInterruptFlags
        );
    }

    /* load the new IDT */
    __asm__ volatile("lidt %0" : : "m"(idt_.idtr));

    TRACE_SUCCESS("IDT initialized");
}
