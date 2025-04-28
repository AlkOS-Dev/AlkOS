#ifndef ALKOS_KERNEL_INCLUDE_HARDWARE_CLOCK_INFRA_HPP_
#define ALKOS_KERNEL_INCLUDE_HARDWARE_CLOCK_INFRA_HPP_

#include <extensions/template_lib.hpp>

namespace hardware
{

struct ClockCallbacks {
};

class ClockRegistry : public template_lib::NoCopy
{
    // ------------------------------
    // Class creation
    // ------------------------------

    public:
    ClockRegistry()  = default;
    ~ClockRegistry() = default;
};

}  // namespace hardware

#endif  // ALKOS_KERNEL_INCLUDE_HARDWARE_CLOCK_INFRA_HPP_
