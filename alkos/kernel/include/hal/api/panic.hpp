#ifndef ALKOS_KERNEL_ABI_PANIC_HPP_
#define ALKOS_KERNEL_ABI_PANIC_HPP_

namespace arch
{

/**
 * @brief Stops the kernel from functioning and disables all necessary devices and processes.
 * @note This function should also dump relevant debug information to the terminal
 *       to help diagnose the issue.
 * @param msg A message providing additional information about the panic.
 */
extern "C" NO_RET void KernelPanic(const char *msg);

}  // namespace arch

#endif  // ALKOS_KERNEL_ABI_PANIC_HPP_
