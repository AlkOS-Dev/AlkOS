#ifndef ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_INTERRUPT_PARAMS_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_INTERRUPT_PARAMS_HPP_

#include <defines.h>
#include <extensions/types.hpp>
#include <hal/api/interrupts_params.hpp>

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

namespace arch
{
static constexpr size_t kNumExceptionHandlers   = 32;
static constexpr size_t kNumHardwareInterrupts  = 32;
static constexpr size_t kNumSoftwareInterrupts  = 32;
static constexpr size_t kNumX86_64CpuExceptions = 32;
static constexpr size_t kNumX86_64Irqs          = 16;

struct ExceptionData : public IsrErrorStackFrame {
};

/* Mapping params */
static constexpr u16 kTimerHwLirq      = 0;
static constexpr u16 kPageFaultExcLirq = 14;

/**
 * @brief x86_64 Page Fault Error Code structure.
 *
 * This structure directly maps to the 64-bit error code pushed
 * onto the stack by the CPU during a page fault.
 *
 * @see Intel 64 and IA-32 Architectures Software Developer's Manual,
 * Volume 3A, Section "Page-Fault Exceptions".
 * https://wiki.osdev.org/Exceptions#Page_Fault
 *
 */
struct PACK PageFaultErrorCode {
    /**
     * @brief Bit 0: Present (P)
     * - 0: The fault was caused by a non-present page. This occurs when the P flag (Present)
     *      in one of the paging-structure entries was 0.
     * - 1: The fault was caused by a page-level protection violation. The page was present,
     *      but the access violated access-rights (e.g., write to a read-only page).
     */
    bool present : 1;
    /**
     * @brief Bit 1: Write/Read (W/R)
     * This flag describes the memory access type that caused the fault.
     * - 0: The access was a read.
     * - 1: The access was a write.
     */
    bool write : 1;
    /**
     * @brief Bit 2: User/Supervisor (U/S)
     * This flag indicates the privilege level at which the fault occurred.
     * - 0: The access was made from supervisor mode (CPL < 3).
     * - 1: The access was made from user mode (CPL = 3).
     */
    bool user : 1;
    /**
     * @brief Bit 3: Reserved Write (RSVD)
     * - 0: The fault was not caused by a reserved bit violation.
     * - 1: The fault was caused by a reserved bit being set to 1 in one of the
     *      paging-structure entries. This can only be set if the 'p' bit (bit 0) is also 1.
     */
    bool reserved_write : 1;
    /**
     * @brief Bit 4: Instruction Fetch (I/D)
     * - 0: The fault was not caused by an instruction fetch.
     * - 1: The fault was caused by an instruction fetch, typically indicating an attempt
     *      to execute code from a non-executable (NX/XD) page when protection is enabled.
     */
    bool instruction_fetch : 1;
    /**
     * @brief Bit 5: Protection Key (PK)
     * - 0: The fault was not caused by a protection-key violation.
     * - 1: The fault was caused by a protection-key violation. This occurs on data accesses
     *      when Memory Protection Keys (MPK) are enabled and the access violates the
     *      permissions defined in the PKRU register (for user mode) or IA32_PKRS MSR
     *      (for supervisor mode).
     */
    bool protection_key : 1;
    /**
     * @brief Bit 6: Shadow Stack (SS)
     * - 0: The fault was not caused by a shadow-stack access.
     * - 1: The fault was caused by a shadow-stack access, related to Control-flow
     *      Enforcement Technology (CET).
     */
    bool shadow_stack : 1;
    /**
     * @brief Bit 7: HLAT Paging
     * - 0: The fault occurred during ordinary paging or was due to access rights.
     * - 1: The fault occurred during translation using HLAT paging structures because a
     *      page was not present or a reserved bit was set.
     */
    bool hlat : 1;
    /**
     * @brief Bits 8-14: Reserved.
     */
    u64 reserved : 7;
    /**
     * @brief Bit 15: SGX (Software Guard Extensions)
     * - 0: The fault is not related to SGX.
     * - 1: The fault resulted from a violation of SGX-specific access-control requirements.
     *      This is set only if the fault is unrelated to ordinary paging (P=1, RSVD=0, PK=0).
     */
    bool sgx : 1;
    u64 reserved2 : 16;
    u64 reserved3 : 32;
};
static_assert(sizeof(PageFaultErrorCode) == sizeof(u64));

}  // namespace arch

#endif  // ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_INTERRUPT_PARAMS_HPP_
