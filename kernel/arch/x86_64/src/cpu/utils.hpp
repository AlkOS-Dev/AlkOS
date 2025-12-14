#ifndef KERNEL_ARCH_X86_64_SRC_CPU_UTILS_HPP_
#define KERNEL_ARCH_X86_64_SRC_CPU_UTILS_HPP_

#include <defines.hpp>
#include <template/rolled_switch.hpp>
#include <types.hpp>

#include "array.hpp"
#include "include/io.hpp"

/**
 * @file arch_utils.hpp
 * @brief x86_64 architecture-specific utility functions and CPU state management
 *
 * Provides low-level CPU control functions and CPU state inspection capabilities
 * for the x86_64 architecture.
 */

/**
 * @brief Disable hardware interrupts
 *
 * Executes the CLI (Clear Interrupt Flag) instruction to prevent
 * hardware interrupts from being serviced.
 */
FAST_CALL void BlockHardwareInterrupts() { __asm__ volatile("cli"); }

/**
 * @brief Enable hardware interrupts
 *
 * Executes the STI (Set Interrupt Flag) instruction to allow
 * hardware interrupts to be serviced.
 */
FAST_CALL void EnableHardwareInterrupts() { __asm__ volatile("sti"); }

/**
 * @brief Halt CPU execution
 *
 * Executes the HLT instruction to stop CPU execution until
 * the next interrupt occurs.
 */
FAST_CALL void HaltCpu() { __asm__ volatile("hlt"); }

/**
 * @brief Infinite loop with CPU halting
 *
 * Enters an infinite loop that continuously halts the CPU.
 * Allows interrupts to wake the CPU.
 */
FAST_CALL NO_RET void OsHang()
{
    while (true) {
        HaltCpu();
    }
}

/**
 * @brief Infinite loop with CPU halting and interrupts disabled
 *
 * Disables interrupts and enters an infinite halt loop.
 * System cannot be woken up from this state.
 */
FAST_CALL void OsHangNoInterrupts()
{
    BlockHardwareInterrupts();
    OsHang();
}

/**
 * @brief Generate software interrupt
 *
 * @param idx Interrupt vector number to generate
 *
 * Executes INT instruction with specified vector number.
 */
template <const u8 idx>
FAST_CALL void InvokeInterrupt()
{
    __asm__ volatile("int %0" : : "N"(idx));
}

FAST_CALL void InvokeInterruptDynamic(const u8 idx)
{
    template_lib::RolledSwitch<u8, 64, 1>(
        []<const u64 idx> {
            return InvokeInterrupt<idx>();
        },
        idx
    );
}

enum RegisterIdx : size_t {
    kRegRax = 0,
    kRegRbx,
    kRegRcx,
    kRegRdx,
    kRegRsi,
    kRegRdi,
    kRegRbp,
    kRegRsp,
    kRegR8,
    kRegR9,
    kRegR10,
    kRegR11,
    kRegR12,
    kRegR13,
    kRegR14,
    kRegR15,
    kRegRip,
    kRegRflags,
    kRegCr0,
    kRegCr2,
    kRegCr3,
    kRegCr4,
    kRegCount
};

union DumpedRegisters {
    struct {
        u64 rax, rbx, rcx, rdx, rsi, rdi, rbp, rsp;
        u64 r8, r9, r10, r11, r12, r13, r14, r15;
        u64 rip;
        u64 rflags;
        u64 cr0, cr2, cr3, cr4;
    };

    std::array<u64, 22> flat;
};

FAST_CALL void DumpRegisters(DumpedRegisters *regs)
{
    __asm__ volatile(
        "movq %%rax, 0x00(%0) \n"
        "movq %%rbx, 0x08(%0) \n"
        "movq %%rcx, 0x10(%0) \n"
        "movq %%rdx, 0x18(%0) \n"
        "movq %%rsi, 0x20(%0) \n"
        "movq %%rdi, 0x28(%0) \n"
        "movq %%rbp, 0x30(%0) \n"
        "movq %%rsp, 0x38(%0) \n"

        "movq %%r8,  0x40(%0) \n"
        "movq %%r9,  0x48(%0) \n"
        "movq %%r10, 0x50(%0) \n"
        "movq %%r11, 0x58(%0) \n"
        "movq %%r12, 0x60(%0) \n"
        "movq %%r13, 0x68(%0) \n"
        "movq %%r14, 0x70(%0) \n"
        "movq %%r15, 0x78(%0) \n"

        "lea (%%rip), %%rax   \n"
        "movq %%rax, 0x80(%0) \n"

        "pushfq               \n"
        "popq %%rax           \n"
        "movq %%rax, 0x88(%0) \n"

        "movq %%cr0, %%rax    \n"
        "movq %%rax, 0x90(%0) \n"

        "movq %%cr2, %%rax    \n"
        "movq %%rax, 0x98(%0) \n"

        "movq %%cr3, %%rax    \n"
        "movq %%rax, 0xA0(%0) \n"

        "movq %%cr4, %%rax    \n"
        "movq %%rax, 0xA8(%0) \n"

        :
        : "r"(regs)
        : "rax", "memory"
    );
}

FAST_CALL void DumpGeneralRegisters(DumpedRegisters *regs)
{
    __asm__ volatile(
        "movq %%rax, 0x00(%0) \n"
        "movq %%rbx, 0x08(%0) \n"
        "movq %%rcx, 0x10(%0) \n"
        "movq %%rdx, 0x18(%0) \n"
        "movq %%rsi, 0x20(%0) \n"
        "movq %%rdi, 0x28(%0) \n"
        "movq %%rbp, 0x30(%0) \n"
        "movq %%rsp, 0x38(%0) \n"

        "movq %%r8,  0x40(%0) \n"
        "movq %%r9,  0x48(%0) \n"
        "movq %%r10, 0x50(%0) \n"
        "movq %%r11, 0x58(%0) \n"
        "movq %%r12, 0x60(%0) \n"
        "movq %%r13, 0x68(%0) \n"
        "movq %%r14, 0x70(%0) \n"
        "movq %%r15, 0x78(%0) \n"

        :
        : "r"(regs)
        : "rax", "memory"
    );
}

void TraceDumpedRegisters(const DumpedRegisters *regs);

#endif  // KERNEL_ARCH_X86_64_SRC_CPU_UTILS_HPP_
