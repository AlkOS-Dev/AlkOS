#ifndef ALKOS_KERNEL_ARCH_X86_64_BOOT_LIB_ELF_ERROR_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_BOOT_LIB_ELF_ERROR_HPP_

namespace Elf64
{

enum class Error {
    InvalidElf,
    UnsupportedMachine,
    NullEntryPoint,
    DynamicHeaderNotFound,
    RelocationTableNotFound,
};

}  // namespace Elf64

#endif  // ALKOS_KERNEL_ARCH_X86_64_BOOT_LIB_ELF_ERROR_HPP_
