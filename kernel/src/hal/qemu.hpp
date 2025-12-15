#ifndef KERNEL_SRC_HAL_QEMU_HPP_
#define KERNEL_SRC_HAL_QEMU_HPP_

#include <hal/impl/qemu.hpp>

namespace hal
{
/* This is plain bad. But it's also quick to implement and easy to delete
 * when time comes for proper abstraction */
WRAP_CALL void QemuShutdown() { arch::QemuShutdown(); }
}  // namespace hal

#endif  // KERNEL_SRC_HAL_QEMU_HPP_
