#ifndef ALKOS_KERNEL_ARCH_X86_64_SRC_INTERRUPTS_IDT_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_SRC_INTERRUPTS_IDT_HPP_

#include <extensions/defines.hpp>
#include "extensions/types.hpp"
#include "interrupts/interrupt_types.hpp"

// ------------------------------
// Crucial defines
// ------------------------------

static constexpr u32 kIdtEntries = 256;

static constexpr u16 kIrq1Offset     = 0x20; /* Start for hardware interrupts */
static constexpr u16 kIrq2Offset     = 0x28;
static constexpr u16 kSpuriousVector = 0xFF; /* Spurious interrupt vector */

static constexpr u8 kExceptionIdx[]{8, 10, 11, 12, 13, 14, 17, 21, 29, 30};
static constexpr size_t kExceptionCount = sizeof(kExceptionIdx) / sizeof(kExceptionIdx[0]);

static const char *kExceptionMsg[]{
    "Double fault",
    "Invalid TSS",
    "Segment not present",
    "Stack-segment fault",
    "General protection fault",
    "Page fault",
    "Alignment check",
    "Control Protection Exception",
    "VMM Communication Exception",
    "Security exception"
};
static constexpr size_t kExceptionMsgCount = sizeof(kExceptionMsg) / sizeof(kExceptionMsg[0]);

static_assert(
    kExceptionCount == kExceptionMsgCount,
    "Exception index and message arrays must have the same size"
);

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

struct PACK IsrStackFrame {
    u64 rip;
    u64 cs;
    u64 rflags;
    u64 rsp;
};

struct PACK IsrErrorStackFrame {
    u64 error_code;
    IsrStackFrame isr_stack_frame;
};

struct Idt {
    /* global structure defining isr specifics for each interrupt signal */
    alignas(32) IdtEntry idt[kIdtEntries]{};

    /* holds information about the idt position in memory */
    Idtr idtr{};
};

// ------------------------------
// Functions
// ------------------------------

const char *GetExceptionMsg(u8 idx);
void DefaultInterruptHandler(u8 idt_idx);
void DefaultExceptionHandler(IsrErrorStackFrame *stack_frame, u8 idt_idx);
void SimpleIrqHandler(intr::LitHwEntry &entry);
void TestIsr(intr::LitSwEntry &entry);
void TimerIsr(intr::LitHwEntry &entry);

#endif  // ALKOS_KERNEL_ARCH_X86_64_SRC_INTERRUPTS_IDT_HPP_
