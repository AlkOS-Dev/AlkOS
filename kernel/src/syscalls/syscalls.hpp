#ifndef KERNEL_SRC_SYSCALLS_SYSCALLS_HPP_
#define KERNEL_SRC_SYSCALLS_SYSCALLS_HPP_

#include <defines.h>
#include <sys/time.h>
#include <types.hpp>

#include "dispatch_table.hpp"
#include "hal/debug_terminal.hpp"
#include "hal/panic.hpp"
#include "modules/timing.hpp"
#include "modules/timing_constants.hpp"

#include <span.hpp>
#include <sys/shell.hpp>

namespace Syscall
{

// ------------------------------
// Panic Syscall
// ------------------------------

/**
 * @brief Trigger a kernel panic with a message
 * @param msg Panic message
 */
NO_RET FORCE_INLINE_F void SysPanic(const char *msg)
{
    hal::KernelPanic(msg);
    __builtin_unreachable();
}

// ------------------------------
// Time Syscalls
// ------------------------------

/**
 * @brief Get clock value
 * @param type Clock type to query
 * @param time Output time value
 * @param time_zone Output timezone
 */
FORCE_INLINE_F void SysGetClockValue(ClockType type, TimeVal *time, Timezone *time_zone)
{
    ASSERT_NOT_NULL(time);
    ASSERT_NOT_NULL(time_zone);

    if (!TimingModule::IsInited()) {
        if (time != nullptr) {
            time->seconds   = 0;
            time->remainder = 0;
        }
        return;
    }

    if (time_zone != nullptr) {
        __platform_get_timezone(time_zone);
    }

    if (time != nullptr) {
        switch (type) {
            case kTimeUtc: {
                time->seconds   = TimingModule::Get().GetSystemTime().Read();
                time->remainder = 0;
            } break;
            case kProcTime: {  // In microseconds
                time->seconds   = 0;
                time->remainder = timing::SystemTime::ReadLifeTimeNs() / 1000;
            } break;
            case kProcTimePrecise: {  // In nanoseconds
                time->seconds   = 0;
                time->remainder = timing::SystemTime::ReadLifeTimeNs();
            } break;
            default:
                R_FAIL_ALWAYS("Provided invalid ClockType!");
        }
    }
}

/**
 * @brief Get timezone information
 * @param time_zone Output timezone
 */
FORCE_INLINE_F void SysGetTimezone(Timezone *time_zone)
{
    ASSERT_NOT_NULL(time_zone);

    ASSERT_TRUE(TimingModule::IsInited(), "Timing module is not initialized");
    *time_zone = TimingModule::Get().GetSystemTime().GetTimezone();
}

/**
 * @brief Get clock ticks per second for a given clock type
 * @param type Clock type
 * @return Ticks per second
 */
FORCE_INLINE_F u64 SysGetClockTicksInSecond(ClockType type)
{
    const auto idx = static_cast<size_t>(type);
    ASSERT(idx != 0 && idx < timing_constants::kClockTicksInSecondSize);

    TODO_USERSPACE
    //    if (idx == 0 || idx >= kResoSize) {
    //        return 0;
    //    }

    return timing_constants::kClockTicksInSecond[idx];
}

// ------------------------------
// Debug IO Syscalls
// ------------------------------

/**
 * @brief Write string to debug output
 * @param buffer String to write (null-terminated)
 */
FORCE_INLINE_F void SysDebugWrite(const char *buffer) { DEBUG_INFO_GENERAL(buffer); }

/**
 * @brief Read line from debug input
 * @param buffer Buffer to store input
 * @param buffer_size Size of buffer
 * @return Number of characters read
 */
FORCE_INLINE_F u64 SysDebugReadLine(char *buffer, size_t buffer_size)
{
    return hal::DebugTerminalReadLine(buffer, buffer_size);
}

// !!! TEMPORARY !!!
FORCE_INLINE_F void SysWriteConsole(const char *buffer)
{
    if (System::g_active_console) {
        System::g_active_console->Write(
            std::span<const byte>(reinterpret_cast<const byte *>(buffer), strlen(buffer))
        );
    }
}

}  // namespace Syscall

#endif  // KERNEL_SRC_SYSCALLS_SYSCALLS_HPP_
