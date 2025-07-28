#include <todo.h>
#include <extensions/time.hpp>
#include <extensions/types.hpp>
#include <time_internal.hpp>

u64 __GetLocalTimezoneOffsetNs()
{
    TODO_TIMEZONES
    /* Hard coded UTC */
    static constexpr u64 kUctOffset2 = 1;

    return kNanosInSecond * kSecondsInHour * kUctOffset2;
}

u64 __GetDstTimezoneOffsetNs()
{
    TODO_TIMEZONES
    /* Hard coded Poland */
    static constexpr u64 kPolandOffset = 1;

    return kNanosInSecond * kSecondsInHour * kPolandOffset;
}
