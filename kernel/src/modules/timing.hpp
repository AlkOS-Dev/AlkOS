#ifndef ALKOS_KERNEL_INCLUDE_MODULES_TIMING_HPP_
#define ALKOS_KERNEL_INCLUDE_MODULES_TIMING_HPP_

#include <extensions/template_lib.hpp>

#include "modules/helpers.hpp"
#include "modules/timing_constants.hpp"
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
};
}  // namespace internal

using TimingModule = template_lib::StaticSingleton<internal::TimingModule>;

#endif  // ALKOS_KERNEL_INCLUDE_MODULES_TIMING_HPP_
