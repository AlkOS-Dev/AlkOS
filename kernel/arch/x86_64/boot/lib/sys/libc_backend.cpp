// This file provides the loader's implementation of the libc platform ABI.
#include <platform.h>

#include <assert.h>
#include <autogen/feature_flags.h>
#include "cpu/utils.hpp"
#include "hw/serial/qemu.hpp"
#include "panic.hpp"

void __platform_panic(const char *msg)
{
    if constexpr (FeatureEnabled<FeatureFlag::kDebugOutput>) {
        KernelPanic(msg);
    }

    OsHang();
}

void __platform_debug_write(const char *buffer)
{
    if constexpr (FeatureEnabled<FeatureFlag::kDebugOutput>) {
        QemuTerminalWriteString(buffer);
    }
}

void __platform_proc_abort() {}

void *__platform_get_heap_start() { return nullptr; }
