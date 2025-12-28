#ifndef KERNEL_SRC_TIME_EVENT_FRAMEWORK_HPP_
#define KERNEL_SRC_TIME_EVENT_FRAMEWORK_HPP_

#include <defines.hpp>
#include <types.hpp>

namespace timing
{

class EventFramework
{
    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    EventFramework()  = default;
    ~EventFramework() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    void InstallInterruptHandler();

    void SetupPeriodic(u64 time_ns);

    // ------------------------------
    // Private methods
    // ------------------------------

    protected:
    // ------------------------------
    // Class fields
    // ------------------------------
};

}  // namespace timing

#endif  // KERNEL_SRC_TIME_EVENT_FRAMEWORK_HPP_
