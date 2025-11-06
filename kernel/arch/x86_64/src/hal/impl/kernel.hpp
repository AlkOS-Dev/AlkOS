#ifndef KERNEL_ARCH_X86_64_SRC_HAL_IMPL_KERNEL_HPP_
#define KERNEL_ARCH_X86_64_SRC_HAL_IMPL_KERNEL_HPP_

#include <defines.h>
#include <hal/api/boot_args.hpp>
#include <types.hpp>

namespace arch
{
struct PACK alignas(64) RawBootArguments : RawBootArgumentsAPI {
    u64 multiboot_info_phys_addr;
    u64 multiboot_header_start_phys_addr;
    u64 multiboot_header_end_phys_addr;
};

void ArchInit(const RawBootArguments &args);
}  // namespace arch

#endif  // KERNEL_ARCH_X86_64_SRC_HAL_IMPL_KERNEL_HPP_
