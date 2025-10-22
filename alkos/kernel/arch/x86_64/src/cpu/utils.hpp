#ifndef ALKOS_KERNEL_ARCH_X86_64_SRC_CPU_UTILS_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_SRC_CPU_UTILS_HPP_

#include <extensions/defines.hpp>
#include <extensions/types.hpp>
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
FAST_CALL void InvokeInterrupt(const u8 idx) { __asm__ volatile("int %0" : : "N"(idx)); }

/**
 * @brief CPU state container structure
 *
 * Holds values of all general-purpose registers for x86_64 CPU.
 */
struct PACK CpuState final {
    enum GeneralPurposeRegisters {
        kRax,
        kRbx,
        kRcx,
        kRdx,
        kRsi,
        kRdi,
        kRbp,
        kRsp,
        kR8,
        kR9,
        kR10,
        kR11,
        kR12,
        kR13,
        kR14,
        kR15,
        kGprLast,
    };

    u64 general_purpose_registers[kGprLast];

    void GetStateDesc(char *buff, size_t buff_size) const;

    void DumpStateDesc() const;
};

[[nodiscard]] CpuState DumpCpuState();

[[nodiscard]] u32 StackLeftBytes();

#endif  // ALKOS_KERNEL_ARCH_X86_64_SRC_CPU_UTILS_HPP_
