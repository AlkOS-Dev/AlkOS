#ifndef ALKOS_BOOT_LIB_ELF_ERROR_HPP_
#define ALKOS_BOOT_LIB_ELF_ERROR_HPP_

namespace Elf64
{

enum class Error {
    InvalidElf,
    UnsupportedMachine,
    NullEntryPoint,
};

}  // namespace Elf64

#endif  // ALKOS_BOOT_LIB_ELF_ERROR_HPP_
