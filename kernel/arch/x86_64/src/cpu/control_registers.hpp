#ifndef ALKOS_KERNEL_ARCH_X86_64_SRC_CPU_CONTROL_REGISTERS_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_SRC_CPU_CONTROL_REGISTERS_HPP_

#include <extensions/concepts.hpp>
#include <extensions/concepts_ext.hpp>
#include <extensions/types.hpp>

/**
 * @file control_registers.hpp
 * @brief Utility to simplify usage of CPU's control registers.
 *
 * @see: Intel 64 and IA-32 Architectures Software Developer's Manual,
 * Volume 3A: System Programming Guide, Part 1. Chapter 2
 */

namespace cpu
{

// clang-format off
struct PACK Cr0 {
    bool ProtectionEnable : 1;      // PE: Enables protected mode.
    bool MonitorCoprocessor : 1;  // MP: Controls interaction of WAIT/FWAIT instructions with TS flag.
    bool Emulation : 1;             // EM: Enables x87 FPU instruction emulation.
    bool TaskSwitched : 1;          // TS: Set on a task switch, causes x87 FPU exception.
    bool ExtensionType : 1;         // ET: Reserved, hardwired to 1.
    bool NumericError : 1;          // NE: Enables native x87 FPU error reporting.
    u64 : 10;                       // Reserved
    bool WriteProtect : 1;          // WP: Prevents supervisor-mode procedures from writing to read-only pages.
    u64 : 1;                        // Reserved
    bool AlignmentMask : 1;         // AM: Enables alignment checking on memory accesses.
    u64 : 10;                       // Reserved
    bool NotWriteThrough : 1;       // NW: Globally enables/disables write-through caching.
    bool CacheDisable : 1;          // CD: Globally enables/disables the memory cache.
    bool Paging : 1;                // PG: Enables paging and the use of the CR3 register.
    u64 : 32;                       // Reserved
};
static_assert(sizeof(Cr0) == 8, "Cr0 structure must be 8 bytes");

struct PACK Cr2 {
    u64 PageFaultLinearAddress;  // Contains the linear address that caused the last page fault.
};
static_assert(sizeof(Cr2) == 8, "Cr2 structure must be 8 bytes");

struct PACK Cr3 {
    u64 : 3;                        // Reserved
    bool PageLevelCacheDisable : 1; // PCD: Page-level Cache Disable.
    bool PageLevelWriteThrough : 1; // PWT: Page-level Write-Through.
    u64 : 7;                        // Reserved
    u64 PageMapLevel4Address : 52;  // Physical address of the PML4 table.
};
static_assert(sizeof(Cr3) == 8, "Cr3 structure must be 8 bytes");

struct PACK Cr4 {
    bool Virtual8086ModeExtensions : 1;           // VME: Virtual-8086 Mode Extensions.
    bool ProtectedModeVirtualInterrupts : 1;    // PVI: Protected-Mode Virtual Interrupts.
    bool TimeStampDisable : 1;                    // TSD: Time Stamp Disable.
    bool DebuggingExtensions : 1;                 // DE: Debugging Extensions.
    bool PageSizeExtension : 1;                   // PSE: Page Size Extension.
    bool PhysicalAddressExtension : 1;            // PAE: Physical Address Extension.
    bool MachineCheckEnable : 1;                  // MCE: Machine-Check Enable.
    bool PageGlobalEnable : 1;                    // PGE: Page Global Enable.
    bool PerformanceMonitoringCounterEnable : 1;  // PCE: Performance-Monitoring Counter Enable.
    bool OsFxsaveFxrstorSupport : 1;            // OSFXSR: Operating System Support for FXSAVE and FXRSTOR instructions.
    bool OsXmmExceptionSupport : 1;             // OSXMMEXCPT: Operating System Support for Unmasked SIMD Floating-Point Exceptions.
    bool UserModeInstructionPrevention : 1;       // UMIP: User-Mode Instruction Prevention.
    u8 : 1;                                      // Reserved
    bool VmxEnable : 1;                           // VMXE: VMX-Enable Bit.
    bool SmxEnable : 1;                           // SMXE: SMX-Enable Bit.
    u8 : 1;                                      // Reserved
    bool FsgsbaseEnable : 1;                      // FSGSBASE: FSGSBASE-Enable Bit.
    bool PcideEnable : 1;                         // PCIDE: PCID-Enable Bit.
    bool OsXsave : 1;                             // OSXSAVE: XSAVE and Processor Extended States-Enable Bit.
    u8 : 1;                                      // Reserved
    bool SupervisorModeExecutionPrevention : 1; // SMEP: Supervisor-Mode Execution Prevention.
    bool SupervisorModeAccessPrevention : 1;    // SMAP: Supervisor-Mode Access Prevention.
    bool ProtectionKeyEnable : 1;                 // PKE: Protection-Key-Enable Bit.
    u64 reserved: 41;                                     // Reserved
};
static_assert(sizeof(Cr4) == 8, "Cr4 structure must be 8 bytes");
// clang-format on

template <typename T>
concept IsControlRegister = concepts_ext::OneOf<T, Cr0, Cr2, Cr3, Cr4>;

template <class T>
    requires IsControlRegister<T>
T GetCR()
{
    u64 value;
    if constexpr (std::is_same_v<T, Cr0>) {
        asm volatile("mov %%cr0, %0" : "=r"(value));
    } else if constexpr (std::is_same_v<T, Cr2>) {
        asm volatile("mov %%cr2, %0" : "=r"(value));
    } else if constexpr (std::is_same_v<T, Cr3>) {
        asm volatile("mov %%cr3, %0" : "=r"(value));
    } else if constexpr (std::is_same_v<T, Cr4>) {
        asm volatile("mov %%cr4, %0" : "=r"(value));
    }

    return *reinterpret_cast<T *>(&value);
}

template <IsControlRegister T>
void SetCR(T value)
{
    if constexpr (std::is_same_v<T, Cr0>) {
        asm volatile("mov %0, %%cr0" : : "r"(value));
    } else if constexpr (std::is_same_v<T, Cr2>) {
        asm volatile("mov %0, %%cr2" : : "r"(value));
    } else if constexpr (std::is_same_v<T, Cr3>) {
        asm volatile("mov %0, %%cr3" : : "r"(value));
    } else if constexpr (std::is_same_v<T, Cr4>) {
        asm volatile("mov %0, %%cr4" : : "r"(value));
    }
}

}  // namespace cpu

#endif  // ALKOS_KERNEL_ARCH_X86_64_SRC_CPU_CONTROL_REGISTERS_HPP_
