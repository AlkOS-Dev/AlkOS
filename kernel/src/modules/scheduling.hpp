#ifndef KERNEL_SRC_MODULES_SCHEDULING_HPP_
#define KERNEL_SRC_MODULES_SCHEDULING_HPP_

#include <template_lib.hpp>

#include "modules/helpers.hpp"
#include "scheduling/processes.hpp"
#include "scheduling/scheduler.hpp"
#include "scheduling/task_mgr.hpp"
#include "scheduling/threads.hpp"

namespace internal
{
class SchedulingModule : template_lib::StaticSingletonHelper
{
    // -------------------------------------
    // Protected singleton constructor
    // -------------------------------------

    protected:
    SchedulingModule() noexcept = default;

    // ------------------------------
    // Module fields
    // ------------------------------

    DEFINE_MODULE_FIELD(Sched, Processes)
    DEFINE_MODULE_FIELD(Sched, Threads)
    DEFINE_MODULE_FIELD(Sched, TaskMgr)

    public:
};
}  // namespace internal

using SchedulingModule = template_lib::StaticSingleton<internal::SchedulingModule>;

#endif  // KERNEL_SRC_MODULES_SCHEDULING_HPP_
