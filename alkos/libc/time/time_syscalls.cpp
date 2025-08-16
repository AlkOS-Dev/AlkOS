#include <assert.h>
#include <sys/calls/time.h>
#include <todo.h>
#include <extensions/time.hpp>

#ifdef __ALKOS_LIBK__

#include <modules/timing.hpp>

// ----------------------------------------
// Internal kernel use implementation
// ----------------------------------------

void GetClockValueSysCall(const ClockType type, TimeVal* time, Timezone* time_zone)
{
    assert(time != nullptr || time_zone != nullptr);

    if (time_zone != nullptr) {
        GetTimezoneSysCall(time_zone);
    }

    if (time != nullptr) {
        switch (type) {
            case kTimeUtc: {
                time->seconds   = TimingModule::Get().GetSystemTime().GetTime();
                time->remainder = 0;
            } break;
            case kProcTime: {  // In microseconds
                time->seconds =
                    TimingModule::Get().GetSystemTime().GetSysLiveTimeNs() / kNanosInSecond;
                time->remainder =
                    (TimingModule::Get().GetSystemTime().GetSysLiveTimeNs() % kNanosInSecond) /
                    1000;
            } break;
            case kProcTimePrecise: {  // In nanoseconds
                time->seconds =
                    TimingModule::Get().GetSystemTime().GetSysLiveTimeNs() / kNanosInSecond;
                time->remainder =
                    TimingModule::Get().GetSystemTime().GetSysLiveTimeNs() % kNanosInSecond;
            } break;
            default:
                R_FAIL_ALWAYS("Provided invalid ClockType!");
        }
    }
}

void GetTimezoneSysCall(Timezone* time_zone)
{
    assert(time_zone != nullptr);
    *time_zone = TimingModule::Get().GetSystemTime().GetTimezone();
}

u64 GetClockTicksInSecondSysCall(ClockType type)
{
    const auto idx = static_cast<size_t>(type);
    ASSERT(idx != 0 && idx < timing_constants::kClockTicksInSecondSize);

    TODO_USERSPACE
    //    if (idx == 0 || idx >= kResoSize) {
    //        return 0;
    //    }

    return timing_constants::kClockTicksInSecond[idx];
}

#else

// ----------------------------------------
// Actual system calls implementation
// ----------------------------------------

TODO_USERSPACE

#endif  // __ALKOS_LIBK__
