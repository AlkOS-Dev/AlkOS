#ifndef KERNEL_ARCH_X86_64_BOOT_LIB_MEM_ERROR_HPP_
#define KERNEL_ARCH_X86_64_BOOT_LIB_MEM_ERROR_HPP_

enum class MemError {
    OutOfMemory,
    InvalidArgument,
};

#endif  // KERNEL_ARCH_X86_64_BOOT_LIB_MEM_ERROR_HPP_
