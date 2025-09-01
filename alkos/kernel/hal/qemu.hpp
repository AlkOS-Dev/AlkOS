#ifndef ALKOS_KERNEL_HAL_QEMU_HPP_
#define ALKOS_KERNEL_HAL_QEMU_HPP_

#include <defines.h>
#include "todo.hpp"

// TODO

/* This is plain bad. But it's also quick to implement and easy to delete
 * when time comes for proper abstraction */

WRAP_CALL void QemuShutdown();

#include <hal/qemu.hpp>

#endif  // ALKOS_KERNEL_HAL_QEMU_HPP_
