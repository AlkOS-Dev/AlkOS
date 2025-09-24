#ifndef ALKOS_KERNEL_ARCH_X86_64_BOOT_LOADER_64_SRC_ERROR_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_BOOT_LOADER_64_SRC_ERROR_HPP_

namespace loader64
{

enum class Error {
    TransitionDataInvalid,
    KernelModuleNotFound,
    ElfBoundsInvalid,
    ElfLoadFailed,
};

}  // namespace loader64

#endif  // ALKOS_KERNEL_ARCH_X86_64_BOOT_LOADER_64_SRC_ERROR_HPP_
