#ifndef ARCH_X86_64_INTERRUPTS_IDT_HPP_
#define ARCH_X86_64_INTERRUPTS_IDT_HPP_

#include <defines.hpp>
#include "extensions/types.hpp"

// ------------------------------
// Crucial defines
// ------------------------------

static constexpr u16 kIrq1Offset = 0x20;
static constexpr u16 kIrq2Offset = 0x28;

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

// ------------------------------
// Functions
// ------------------------------

void IdtInit();

void LogIrqReceived(void *stack_frame, u8 idt_idx);

const char *GetExceptionMsg(u8 idx);

#endif  // ARCH_X86_64_INTERRUPTS_IDT_HPP_
