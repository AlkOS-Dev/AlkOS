#include <debug.hpp>
#include <time/daytime.hpp>
#include <timers.hpp>

DayTime::DayTime()
{
    TRACE_INFO("DayTime::DayTime()");
    time_ = QuerySystemTime();
}
