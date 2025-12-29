#ifndef KERNEL_SRC_MODULES_TIMING_HPP_
#define KERNEL_SRC_MODULES_TIMING_HPP_

#include <template_lib.hpp>

#include "modules/helpers.hpp"
#include "modules/timing_constants.hpp"
#include "time/event_framework.hpp"
#include "time/system_time.hpp"

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
    // ------------------------------
    // Module fields
    // ------------------------------

    DEFINE_MODULE_FIELD(timing, SystemTime)
    DEFINE_MODULE_FIELD(timing, EventFramework)
};
}  // namespace internal

using TimingModule = template_lib::StaticSingleton<internal::TimingModule>;

#endif  // KERNEL_SRC_MODULES_TIMING_HPP_
