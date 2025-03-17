#ifndef ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_ARCH_UTILS_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_ARCH_UTILS_HPP_

#include <defines.hpp>
#include <io.hpp>
#include "extensions/types.hpp"

/**
 * @file arch_utils.hpp
 * @brief x86_64 architecture-specific utility functions and CPU state management
 *
 * Provides low-level CPU control functions and CPU state inspection capabilities
 * for the x86_64 architecture.
 */

/**
 * @brief Check if CPUID instruction is supported
 *
 *  @return true if CPUID instruction is supported, false otherwise
 */
FAST_CALL bool IsCPUIDSupported()
{
    u64 gpr1, gpr2;
    __asm__ volatile(
        "pushf\n\t"
        "pushf\n\t"
        "pop %0\n\t"
        "mov %0, %1\n\t"
        "xor %2, %0\n\t"
        "push %0\n\t"
        "popf\n\t"
        "pushf\n\t"
        "pop %0\n\t"
        "popf\n\t"
        : "=&r"(gpr1), "=&r"(gpr2)
        : "i"(0x0000000000200000)
    );

    return (gpr1 ^ gpr2) & 0x0000000000200000;  // Check if ID bit changed
}

/**
 * @brief Get maximum CPUID leaf supported by the processor
 *
 * @param leaf CPUID leaf to query
 *
 * @remark Doesn't check if CPUID instruction is supported.
 */
FAST_CALL void __CPUID(u32 leaf, u32& eax, u32& ebx, u32& ecx, u32& edx)
{
    __asm__ volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(leaf), "c"(0));
}

/**
 * @brief Extended CPUID query with level and sublevel
 *
 * @param leaf CPUID leaf to query
 * @param subleaf CPUID subleaf to query
 *
 * @remark Doesn't check if CPUID instruction is supported.
 */
FAST_CALL void __CPUID(u32 leaf, u32 subleaf, u32& eax, u32& ebx, u32& ecx, u32& edx)
{
    __asm__ volatile("cpuid"
                     : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                     : "a"(leaf), "c"(subleaf));
}

/**
 * @brief Get maximum CPUID level supported by the processor
 *
 * @param extension CPUID extension level to query, can be either 0x0 or 0x80000000
 *
 * @remark Doesn't check if CPUID instruction is supported.
 */
FAST_CALL u32 __MaxCPUIDLeaf(u32 extension)
{
    u32 eax, ebx, ecx, edx;
    __CPUID(extension, eax, ebx, ecx, edx);
    return eax;
}

/**
 * @brief Get maximum CPUID leaf supported by the processor
 *
 * @param leaf CPUID leaf to query
 * @param regs Array to store CPUID registers
 *
 * @return true if CPUID instruction is supported and the query was successful, false otherwise
 */
FAST_CALL bool CPUID(u32 leaf, u32 (&regs)[4])
{
    if (!IsCPUIDSupported() || leaf > __MaxCPUIDLeaf(leaf & 0x80000000)) {
        return false;
    }
    __CPUID(leaf, regs[0], regs[1], regs[2], regs[3]);
    return true;
}

/**
 * @brief Extended CPUID query with level and sublevel
 *
 * @param leaf CPUID leaf to query
 * @param subleaf CPUID subleaf to query
 *
 * @return true if CPUID instruction is supported and the query was successful, false otherwise
 */
FAST_CALL bool CPUID(u32 leaf, u32 subleaf, u32 (&regs)[4])
{
    if (!IsCPUIDSupported() || leaf > __MaxCPUIDLeaf(leaf & 0x80000000)) {
        return false;
    }
    __CPUID(leaf, subleaf, regs[0], regs[1], regs[2], regs[3]);
    return true;
}

/**
 * @brief Flush CPU cache lines
 *
 * Writes back all modified cache lines in the processor’s internal cache to main
 * memory and invalidates (flushes) the internal caches and flush external caches asynchronously.
 */
FAST_CALL void FlushCPUCache() { __asm__ volatile("wbinvd"); }

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
 * @brief Shut down QEMU emulator
 *
 * Writes shutdown command to QEMU exit port and halts CPU.
 * Only works when running under QEMU.
 */
FAST_CALL NO_RET void QemuShutdown()
{
    outw(0x604, 0x2000);
    OsHang();
}

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

    void GetStateDesc(char* buff, size_t buff_size) const;

    void DumpStateDesc() const;
};

[[nodiscard]] CpuState DumpCpuState();

#endif  // ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_ARCH_UTILS_HPP_
