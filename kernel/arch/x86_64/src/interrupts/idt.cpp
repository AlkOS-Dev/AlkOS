#include "interrupts/idt.hpp"
#include <defines.hpp>

#include <hal/debug.hpp>
#include <hal/panic.hpp>  // I dislike this import architecturally, but let it be for now
#include <modules/timing.hpp>

#include "cpu/gdt.hpp"
#include "cpu/utils.hpp"
#include "hal/interrupts.hpp"
#include "interrupts/interrupt_types.hpp"

#include <assert.h>
#include <bit.hpp>
#include <hal/terminal.hpp>

#include "trace_framework.hpp"

static constexpr u32 kStubTableSize = 144;

/**
 * Flags for ISRs (Interrupt Service Routines):
 * - 0xE: 64-bit Interrupt Gate (0xF for 64-bit Trap Gate)
 * - Bit 4: Reserved (must always be 0)
 * - Bits 5-6: 2-bit value defining the allowed CPU Privilege Levels (CPL)
 *   that can access this interrupt via the INT instruction. Hardware interrupts ignore this.
 * - Bit 7: Present bit (must be set to 1 for the descriptor to be valid).
 */
static constexpr IdtEntryFlags kTrapFlags = {
    .type = IdtGateType::k64BitTrapGate, .zero = 0, .dpl = IdtPrivilegeLevel::kRing0, .present = 1
};
static constexpr IdtEntryFlags kInterruptFlags = {
    .type    = IdtGateType::k64BitInterruptGate,
    .zero    = 0,
    .dpl     = IdtPrivilegeLevel::kRing0,
    .present = 1
};
static constexpr IdtEntryFlags kSyscallFlags = {
    .type    = IdtGateType::k64BitInterruptGate,
    .zero    = 0,
    .dpl     = IdtPrivilegeLevel::kRing3,
    .present = 1
};

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
NO_RET Sched::Thread *DefaultInterruptHandler(const u8 idt_idx)
{
    hal::KernelPanicFormat("Received unsupported interrupt with code: %hhu\n", idt_idx);
    return nullptr;
}

static void FormatIsrRegisters(
    const IsrErrorStackFrame *stack_frame, char *buff, const size_t buff_size
)
{
    static constexpr const char *kRegNames[] = {"rip", "rflags", "rsp", "rax", "rbx", "rcx",
                                                "rdx", "rsi",    "rdi", "rbp", "r8",  "r9",
                                                "r10", "r11",    "r12", "r13", "r14", "r15"};

    const uint64_t reg_values[] = {
        stack_frame->isr_stack_frame.rip, stack_frame->isr_stack_frame.rflags,
        stack_frame->isr_stack_frame.rsp, stack_frame->registers.rax,
        stack_frame->registers.rbx,       stack_frame->registers.rcx,
        stack_frame->registers.rdx,       stack_frame->registers.rsi,
        stack_frame->registers.rdi,       stack_frame->registers.rbp,
        stack_frame->registers.r8,        stack_frame->registers.r9,
        stack_frame->registers.r10,       stack_frame->registers.r11,
        stack_frame->registers.r12,       stack_frame->registers.r13,
        stack_frame->registers.r14,       stack_frame->registers.r15
    };

    size_t offset             = 0;
    constexpr size_t num_regs = sizeof(reg_values) / sizeof(reg_values[0]);
    for (size_t i = 0; i < num_regs && offset < buff_size; ++i) {
        int written = snprintf(
            buff + offset, buff_size - offset, "%s: 0x%016llx\n", kRegNames[i],
            static_cast<unsigned long long>(reg_values[i])
        );
        ASSERT_GT(written, 0);
        ASSERT_LT(offset + written, buff_size);
        offset += written;
    }
}

NO_RET FAST_CALL void DefaultExceptionHandler(
    const IsrErrorStackFrame *stack_frame, const u8 idt_idx
)
{
    static constexpr size_t kStateMsgSize = 1024;

    char state_buffer[kStateMsgSize];
    FormatIsrRegisters(stack_frame, state_buffer, kStateMsgSize);

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

Sched::Thread *DefaultExceptionHandler(intr::LitExcEntry &entry, hal::ExceptionData *data)
{
    DefaultExceptionHandler(data, entry.hardware_irq);
    return nullptr;
}

FAST_CALL void LogIrqReceived(const u16 idt_idx, const u16 lirq)
{
    TRACE_FREQ_INFO_INTERRUPTS("Received interrupt with idt idx: %hu and lirq: %hu", idt_idx, lirq);
}

Sched::Thread *SimpleIrqHandler(intr::LitHwEntry &entry)
{
    LogIrqReceived(entry.hardware_irq, entry.logical_irq);
    return nullptr;
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

static void IdtSetDescriptor(Idt &idt, const u8 idx, const u64 isr, const IdtEntryFlags flags)
{
    IdtEntry &entry = idt.idt[idx];

    entry.isr_low    = isr & kBitMask16;
    entry.kernel_cs  = cpu::GDT::kKernelCodeSelector;
    entry.ist        = 0;
    entry.attributes = flags;
    entry.isr_mid    = (isr >> 16) & kBitMask16;
    entry.isr_high   = (isr >> 32) & kBitMask32;
    entry.reserved   = 0;
}

void arch::Interrupts::InitializeDefaultIdt_()
{
    R_ASSERT_LT(cpu::GDT::kKernelCodeSelector, static_cast<u32>(UINT16_MAX));
    R_ASSERT_NEQ(static_cast<u32>(0), cpu::GDT::kKernelCodeSelector);

    idt_.idtr.base  = reinterpret_cast<uintptr_t>(idt_.idt);
    idt_.idtr.limit = static_cast<u16>(sizeof(IdtEntry)) * kIdtEntries - 1;

    for (u8 idx = 0; idx < kStubTableSize; ++idx) {
        IdtSetDescriptor(
            idt_, idx, reinterpret_cast<u64>(IsrWrapperTable[idx]),
            IsTrapEntry(idx) ? kTrapFlags : kInterruptFlags
        );
    }

    // Set syscall interrupt
    IdtSetDescriptor(
        idt_, kSyscallVector, reinterpret_cast<u64>(IsrWrapperTable[kSyscallVector]), kSyscallFlags
    );

    /* load the new IDT */
    __asm__ volatile("lidt %0" : : "m"(idt_.idtr));

    DEBUG_INFO_INTERRUPTS("IDT initialized");
}
