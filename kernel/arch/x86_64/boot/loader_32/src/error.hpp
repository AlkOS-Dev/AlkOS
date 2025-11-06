#ifndef ALKOS_KERNEL_ARCH_X86_64_BOOT_LOADER_32_SRC_ERROR_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_BOOT_LOADER_32_SRC_ERROR_HPP_

namespace loader32
{

enum class Error {
    MultibootCheckFailed,
    CpuIdCheckFailed,
    LongModeCheckFailed,
    Loader64ModuleNotFound,
    ElfLoadFailed,
    MmapTagNotFound,
};

}  // namespace loader32

#endif  // ALKOS_KERNEL_ARCH_X86_64_BOOT_LOADER_32_SRC_ERROR_HPP_
