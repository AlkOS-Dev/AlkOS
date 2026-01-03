#ifndef KERNEL_SRC_SYSCALLS_SYSCALLS_HPP_
#define KERNEL_SRC_SYSCALLS_SYSCALLS_HPP_

#include <defines.h>
#include <types.h>

#include "dispatch_table.hpp"
#include "hal/debug_terminal.hpp"
#include "hal/panic.hpp"
#include "modules/timing.hpp"
#include "modules/timing_constants.hpp"
#include "trace_framework.hpp"

#include "hardware/core_local.hpp"

#include <span.hpp>
#include <sys/shell.hpp>

/* Syscalls */

#include "calls/fd.hpp"
#include "calls/io.hpp"
#include "calls/panic.hpp"
#include "calls/proc.hpp"
#include "calls/thread.hpp"
#include "calls/time.hpp"
#include "calls/video.hpp"

#endif  // KERNEL_SRC_SYSCALLS_SYSCALLS_HPP_
