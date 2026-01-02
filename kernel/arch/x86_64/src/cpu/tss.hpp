#ifndef KERNEL_ARCH_X86_64_SRC_CPU_TSS_HPP_
#define KERNEL_ARCH_X86_64_SRC_CPU_TSS_HPP_

#include <types.h>
#include <defines.hpp>

namespace cpu
{
struct PACK TSS {
    u32 reserved0;
    u64 rsp0;
    u64 rsp1;
    u64 rsp2;
    u64 reserved1;
    u64 ist[7];
    u64 reserved2;
    u16 reserved3;
    u16 iopb_offset;
};

FAST_CALL void LoadTss(u16 selector) { __asm__ volatile("ltr %0" : : "r"(selector) : "memory"); }

}  // namespace cpu

#endif  // KERNEL_ARCH_X86_64_SRC_CPU_TSS_HPP_
