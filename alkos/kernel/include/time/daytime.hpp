#ifndef ALKOS_INCLUDE_TIME_DAYTIME_HPP_
#define ALKOS_INCLUDE_TIME_DAYTIME_HPP_

#include <time.h>

class DayTime
{
    public:
    DayTime();

    FORCE_INLINE_F time_t GetTime() const noexcept { return time_; }

    private:
    time_t time_;
};

#endif  // ALKOS_INCLUDE_TIME_DAYTIME_HPP_
