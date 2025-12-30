#ifndef KERNEL_SRC_SCHEDULING_ERROR_HPP_
#define KERNEL_SRC_SCHEDULING_ERROR_HPP_

#include <assert.h>

namespace Sched
{

enum class Error {
    OutOfMemory,
    ExceededMaxAllowedInstances,
    ProcessNotFound,
    ThreadNotFound,
    ProcessNameTooLong,
};

}  // namespace Sched

static constexpr const char *to_string(const Sched::Error &error)
{
    switch (error) {
        case Sched::Error::OutOfMemory:
            return "OutOfMemory";
        case Sched::Error::ExceededMaxAllowedInstances:
            return "ExceededMaxAllowedInstances";
        case Sched::Error::ProcessNotFound:
            return "ProcessNotFound";
        case Sched::Error::ThreadNotFound:
            return "ThreadNotFound";
        case Sched::Error::ProcessNameTooLong:
            return "ProcessNameTooLong";
    }

    return "unknown error";
}

#endif  // KERNEL_SRC_SCHEDULING_ERROR_HPP_
