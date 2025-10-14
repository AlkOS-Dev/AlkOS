#ifndef ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_PANIC_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_PANIC_HPP_

namespace arch
{
extern "C" NO_RET void KernelPanic(const char *msg);
}  // namespace arch

#endif  // ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_PANIC_HPP_
