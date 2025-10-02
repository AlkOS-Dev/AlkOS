#include "cpu/utils.hpp"

#include <assert.h>
#include <stdio.h>
#include <hal/terminal.hpp>

/**
 * @brief Capture current CPU register state
 *
 * @return CpuState structure containing current register values
 */
CpuState DumpCpuState()
{
    CpuState cpu_state{};

    __asm__ volatile("movq %%rax, %0" : "=m"(cpu_state.general_purpose_registers[CpuState::kRax]));
    __asm__ volatile("movq %%rbx, %0" : "=m"(cpu_state.general_purpose_registers[CpuState::kRbx]));
    __asm__ volatile("movq %%rcx, %0" : "=m"(cpu_state.general_purpose_registers[CpuState::kRcx]));
    __asm__ volatile("movq %%rdx, %0" : "=m"(cpu_state.general_purpose_registers[CpuState::kRdx]));
    __asm__ volatile("movq %%rsi, %0" : "=m"(cpu_state.general_purpose_registers[CpuState::kRsi]));
    __asm__ volatile("movq %%rdi, %0" : "=m"(cpu_state.general_purpose_registers[CpuState::kRdi]));
    __asm__ volatile("movq %%rbp, %0" : "=m"(cpu_state.general_purpose_registers[CpuState::kRbp]));
    __asm__ volatile("movq %%rsp, %0" : "=m"(cpu_state.general_purpose_registers[CpuState::kRsp]));
    __asm__ volatile("movq %%r8, %0" : "=m"(cpu_state.general_purpose_registers[CpuState::kR8]));
    __asm__ volatile("movq %%r9, %0" : "=m"(cpu_state.general_purpose_registers[CpuState::kR9]));
    __asm__ volatile("movq %%r10, %0" : "=m"(cpu_state.general_purpose_registers[CpuState::kR10]));
    __asm__ volatile("movq %%r11, %0" : "=m"(cpu_state.general_purpose_registers[CpuState::kR11]));
    __asm__ volatile("movq %%r12, %0" : "=m"(cpu_state.general_purpose_registers[CpuState::kR12]));
    __asm__ volatile("movq %%r13, %0" : "=m"(cpu_state.general_purpose_registers[CpuState::kR13]));
    __asm__ volatile("movq %%r14, %0" : "=m"(cpu_state.general_purpose_registers[CpuState::kR14]));
    __asm__ volatile("movq %%r15, %0" : "=m"(cpu_state.general_purpose_registers[CpuState::kR15]));

    return cpu_state;
}

/**
 * Simply formats the CPU state into a human-readable string.
 *
 * @param buff - buffer to write to
 * @param buff_size - size of buffer
 */
void CpuState::GetStateDesc(char *buff, const size_t buff_size) const
{
    static constexpr const char *kRegNames[]{"rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                                             "rbp", "rsp", "r8",  "r9",  "r10", "r11",
                                             "r12", "r13", "r14", "r15"};

    size_t offset = 0;

    for (int i = 0; i < kGprLast && offset < buff_size; ++i) {
        const int written = snprintf(
            buff + offset, buff_size - offset, "%s: 0x%016llx\n", kRegNames[i],
            general_purpose_registers[i]
        );

        offset += written;
        ASSERT_NEQ(offset, buff_size);
    }
}

/**
 * @brief Print formatted dump of CPU register state
 *
 * Outputs current values of all general-purpose registers
 * to the terminal in a human-readable format.
 */
void CpuState::DumpStateDesc() const
{
    static constexpr size_t kSizeBuff = 2048;
    char buff[kSizeBuff];

    GetStateDesc(buff, kSizeBuff);
    hal::TerminalWriteString(buff);
}
