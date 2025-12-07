#ifndef KERNEL_ARCH_X86_64_SRC_HAL_IMPL_KERNEL_HPP_
#define KERNEL_ARCH_X86_64_SRC_HAL_IMPL_KERNEL_HPP_

#include <defines.h>
#include <hal/api/boot_args.hpp>
#include <types.hpp>

namespace arch
{
struct PACK alignas(64) RawBootArguments : RawBootArgumentsAPI {
    static constexpr size_t kMaxRsdpStructSize = 36;
    u8 rsdp_struct[kMaxRsdpStructSize];
    bool is_acpi1;
};

void ArchInit(const RawBootArguments &args);
}  // namespace arch

#endif  // KERNEL_ARCH_X86_64_SRC_HAL_IMPL_KERNEL_HPP_
