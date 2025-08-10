#ifndef ALKOS_KERNEL_INCLUDE_MODULES_TIMING_HPP_
#define ALKOS_KERNEL_INCLUDE_MODULES_TIMING_HPP_

#include <extensions/template_lib.hpp>

#include "modules/helpers.hpp"
#include "modules/timing_constants.hpp"
#include "time/daytime.hpp"

namespace internal
{
class TimingModule : template_lib::StaticSingletonHelper
{
    // -------------------------------------
    // Protected singleton constructor
    // -------------------------------------

    protected:
    TimingModule() noexcept;

    // ------------------------------
    // Settings events
    // ------------------------------

    public:
    static void OnIsUtcChanged() noexcept;

    // ------------------------------
    // Module fields
    // ------------------------------

    DEFINE_MODULE_FIELD(timing, DayTime)
};
}  // namespace internal

using TimingModule = template_lib::StaticSingleton<internal::TimingModule>;

#endif  // ALKOS_KERNEL_INCLUDE_MODULES_TIMING_HPP_
