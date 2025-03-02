#ifndef ALKOS_INCLUDE_MODULES_TIMING_HPP_
#define ALKOS_INCLUDE_MODULES_TIMING_HPP_

#include <extensions/template_lib.hpp>

#include <time/daytime.hpp>

namespace internal
{
class TimingModule : TemplateLib::StaticSingletonHelper
{
    protected:
    TimingModule() noexcept;

    public:
    FORCE_INLINE_F DayTime& GetTime() noexcept { return day_time_; }

    private:
    DayTime day_time_{};
};
}  // namespace internal

using TimingModule = TemplateLib::StaticSingleton<internal::TimingModule>;

#endif  // ALKOS_INCLUDE_MODULES_TIMING_HPP_
