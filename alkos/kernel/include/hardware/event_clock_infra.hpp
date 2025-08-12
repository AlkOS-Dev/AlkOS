#ifndef ALKOS_KERNEL_INCLUDE_HARDWARE_EVENT_CLOCK_INFRA_HPP_
#define ALKOS_KERNEL_INCLUDE_HARDWARE_EVENT_CLOCK_INFRA_HPP_

#include <extensions/data_structures/data_structures.hpp>

namespace hardware
{
struct EventClockRegistryEntry {
    struct ClockCallbacks {
        int (*enable)(EventClockRegistryEntry* clock_entry);
        int (*disable)(EventClockRegistryEntry* clock_entry);
    };
};

static constexpr size_t kMaxEventClocks = 8;
using EventClockRegistry = data_structures::StaticVector<EventClockRegistryEntry, kMaxEventClocks>;

}  // namespace hardware

#endif  // ALKOS_KERNEL_INCLUDE_HARDWARE_EVENT_CLOCK_INFRA_HPP_
