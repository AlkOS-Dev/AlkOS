#include <debug.hpp>
#include <time/daytime.hpp>
#include <timers.hpp>

// ------------------------------
// Implementations
// ------------------------------

DayTime::DayTime()
{
    TRACE_INFO("DayTime::DayTime()");
    SyncWithHardware();
}

void DayTime::SyncWithHardware()
{
    time_ = QuerySystemTime();
    TRACE_INFO("Synced system time with hardware: %lu", time_);
}
