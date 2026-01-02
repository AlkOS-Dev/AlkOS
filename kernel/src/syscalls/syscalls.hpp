#ifndef KERNEL_SRC_SYSCALLS_SYSCALLS_HPP_
#define KERNEL_SRC_SYSCALLS_SYSCALLS_HPP_

/**
 * @brief This file collects all the system calls available in the kernel.
 *
 * Calls should be specified in other header files and then included here.
 */

#include "calls/fd.hpp"
#include "calls/io.hpp"
#include "calls/panic.hpp"
#include "calls/proc.hpp"
#include "calls/thread.hpp"
#include "calls/time.hpp"

#endif  // KERNEL_SRC_SYSCALLS_SYSCALLS_HPP_
