#ifndef ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_QEMU_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_QEMU_HPP_

#include <extensions/defines.hpp>
#include "cpu/utils.hpp"
#include "include/io.hpp"
#include "todo.hpp"

// TODO

WRAP_CALL void QemuShutdown()
{
    outw(0x604, 0x2000);
    OsHang();
}

#endif  // ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_QEMU_HPP_
