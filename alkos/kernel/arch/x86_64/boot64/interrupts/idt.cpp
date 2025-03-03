/* internal includes */
#include <assert.h>
#include <memory.h>
#include <arch_utils.hpp>
#include <bit.hpp>
#include <defines.hpp>
#include <extensions/debug.hpp>
#include <interrupts/idt.hpp>
#include <panic.hpp>

/* crucial defines */
static constexpr u32 kStubTableSize = 64;
static constexpr u32 kIdtEntries    = 256;

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
// Data layout
// ------------------------------

/**
 * @brief Data layout of x86_64 interrupt service routines, refer to intel manual for details
 */
struct PACK IdtEntry {
    u16 isr_low;    // The lower 16 bits of the ISR's address
    u16 kernel_cs;  // The GDT segment selector that the CPU will load into CS before calling the
    // ISR
    u8 ist;         // The IST in the TSS that the CPU will load into RSP; set to zero for now
    u8 attributes;  // Type and attributes; see the IDT page
    u16 isr_mid;    // The higher 16 bits of the lower 32 bits of the ISR's address
    u32 isr_high;   // The higher 32 bits of the ISR's address
    u32 reserved;   // Set to zero
};

/**
 * @brief Structure describing Idt position in memory
 */
struct PACK Idtr {
    u16 limit;
    u64 base;
};

// ------------------------------
// Global data
// ------------------------------

/* vector holding bool to detect double assignment */
alignas(32) static bool g_idtGuards[kIdtEntries];

/* global structure defining isr specifics for each interrupt signal */
alignas(32) static IdtEntry g_idt[kIdtEntries];

/* holds information about the idt position in memory */
static Idtr g_idtr;

// ------------------------------
// Isrs
// ------------------------------

/**
 * @brief This handler is called when an unknown interrupt occurs.
 * Executes KernelPanic.
 *
 * @param idt_idx index of interrupt triggered.
 */
extern "C" NO_RET void DefaultInterruptHandler(const u8 idt_idx)
{
    KernelPanicFormat("Received unsupported interrupt with code: %hhu\n", idt_idx);
}

extern "C" NO_RET void DefaultExceptionHandler(IsrErrorStackFrame *stack_frame, const u8 idt_idx)
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

    const char *exception_msg = GetExceptionMsg(idt_idx);
    R_ASSERT_NOT_NULL(exception_msg);

    KernelPanicFormat(
        "Received exception: %d (%s)\n"
        "And error: %llu\n"
        "At instruction address: 0x%016llx\n"
        "%s"
        "RFLAGS: %0x%016llx\n",
        idt_idx, exception_msg, stack_frame->error_code, stack_frame->isr_stack_frame.rip,
        state_buffer, stack_frame->isr_stack_frame.rflags
    );
}

/**
 * @brief Logs the received interrupt.
 *
 * @param stack_frame Pointer to the ISR stack frame.
 * @param idt_idx index of interrupt triggered.
 */
void LogIrqReceived([[maybe_unused]] void *stack_frame, const u8 idt_idx)
{
    TRACE_INFO("Received interrupt with idx: %hhu\n", idt_idx);
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

static void IdtSetDescriptor(const u8 idx, const u64 isr, const u8 flags)
{
    R_ASSERT_FALSE(g_idtGuards[idx]);

    IdtEntry &entry = g_idt[idx];

    entry.isr_low    = isr & kBitMask16;
    entry.kernel_cs  = kKernelCodeOffset;
    entry.ist        = 0;
    entry.attributes = flags;
    entry.isr_mid    = (isr >> 16) & kBitMask16;
    entry.isr_high   = (isr >> 32) & kBitMask32;
    entry.reserved   = 0;

    g_idtGuards[idx] = true;
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

void IdtInit()
{
    R_ASSERT_LT(kKernelCodeOffset, static_cast<u32>(UINT16_MAX));
    R_ASSERT_NEQ(static_cast<u32>(0), kKernelCodeOffset);

    g_idtr.base  = reinterpret_cast<uintptr_t>(g_idt);
    g_idtr.limit = static_cast<u16>(sizeof(IdtEntry)) * kIdtEntries - 1;

    /* cleanup flag vector */
    memset(g_idtGuards, 0, sizeof(g_idtGuards));

    /* zero IDT */
    memset(g_idt, 0, sizeof(g_idt));

    for (u8 idx = 0; idx < kStubTableSize; ++idx) {
        IdtSetDescriptor(
            idx, reinterpret_cast<u64>(IsrWrapperTable[idx]),
            IsTrapEntry(idx) ? kTrapFlags : kInterruptFlags
        );
    }

    /* load the new IDT */
    __asm__ volatile("lidt %0" : : "m"(g_idtr));

    TRACE_SUCCESS("IDT initialized");
}
