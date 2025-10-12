#ifndef ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_KERNEL_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_KERNEL_HPP_

#include <hal/api/kernel.hpp>

namespace arch
{
struct PACK KernelArguments : KernelArgumentsAPI {
    u64 multiboot_info_addr;
    u64 multiboot_header_start_addr;
    u64 multiboot_header_end_addr;
};

void ArchInit(const KernelArguments& args);
}  // namespace arch

#endif  // ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_KERNEL_HPP_
