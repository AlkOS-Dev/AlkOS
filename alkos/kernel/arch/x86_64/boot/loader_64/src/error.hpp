#ifndef ALKOS_BOOT_LOADER_64_ERROR_HPP_
#define ALKOS_BOOT_LOADER_64_ERROR_HPP_

namespace loader64
{

enum class Error {
    TransitionDataInvalid,
    KernelModuleNotFound,
    ElfBoundsInvalid,
    ElfLoadFailed,
};

}  // namespace loader64

#endif  // ALKOS_BOOT_LOADER_64_ERROR_HPP_
