#ifndef ALKOS_KERNEL_INCLUDE_HARDWARE_CLOCK_INFRA_HPP_
#define ALKOS_KERNEL_INCLUDE_HARDWARE_CLOCK_INFRA_HPP_

#include <extensions/data_structures/data_structures.hpp>

namespace hardware
{

struct ClockRegistryEntry {
    struct ClockCallbacks {
        int (*enable)(ClockRegistryEntry* clock_entry);
        int (*disable)(ClockRegistryEntry* clock_entry);
    };
};

static constexpr size_t kMaxClocks = 8;
using ClockRegistry                = data_structures::StaticVector<ClockRegistryEntry, kMaxClocks>;

}  // namespace hardware

#endif  // ALKOS_KERNEL_INCLUDE_HARDWARE_CLOCK_INFRA_HPP_
