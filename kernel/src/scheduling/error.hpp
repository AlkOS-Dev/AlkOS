#ifndef KERNEL_SRC_SCHEDULING_ERROR_HPP_
#define KERNEL_SRC_SCHEDULING_ERROR_HPP_

namespace Sched
{

enum class Error {
    OutOfMemory,
    ExceededMaxAllowedInstances,
};
}  // namespace Sched

#endif  // KERNEL_SRC_SCHEDULING_ERROR_HPP_
