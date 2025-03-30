#ifndef ALKOS_INCLUDE_MODULES_TIMING_HPP_
#define ALKOS_INCLUDE_MODULES_TIMING_HPP_

#include <extensions/template_lib.hpp>
#include <modules/timing_constants.hpp>
#include <time/daytime.hpp>

namespace internal
{
class TimingModule : TemplateLib::StaticSingletonHelper
{
    // -------------------------------------
    // Protected singleton constructor
    // -------------------------------------

    protected:
    TimingModule() noexcept;

    // ------------------------------
    // Getters
    // ------------------------------

    public:
    FORCE_INLINE_F timing::DayTime& GetDayTime() noexcept { return day_time_; }

    // ------------------------------
    // Settings events
    // ------------------------------

    static void OnIsUtcChanged() noexcept;

    // ------------------------------
    // Module fields
    // ------------------------------

    private:
    timing::DayTime day_time_{};
};
}  // namespace internal

using TimingModule = TemplateLib::StaticSingleton<internal::TimingModule>;

#endif  // ALKOS_INCLUDE_MODULES_TIMING_HPP_
