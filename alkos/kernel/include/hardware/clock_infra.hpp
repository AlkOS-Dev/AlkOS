#ifndef ALKOS_KERNEL_INCLUDE_HARDWARE_CLOCK_INFRA_HPP_
#define ALKOS_KERNEL_INCLUDE_HARDWARE_CLOCK_INFRA_HPP_

#include <extensions/data_structures/data_structures.hpp>

namespace hardware
{

struct ClockRegistryEntry : data_structures::RegistryEntry {
    /* Clock numbers */
    u64 frequency_kHz;               // Frequency in kHz
    u64 uncertainty_margin_per_sec;  // Uncertainty margin in femto seconds per second

    /* Callbacks */
    u64 (*read)(ClockRegistryEntry* clock_entry);
    u64 (*read_femto_seconds)(ClockRegistryEntry* clock_entry);
    bool (*enable_device)(ClockRegistryEntry* clock_entry);
    bool (*disable_device)(ClockRegistryEntry* clock_entry);
    void (*stop_counter)(ClockRegistryEntry* clock_entry);
    void (*resume_counter)(ClockRegistryEntry* clock_entry);
};

static constexpr size_t kMaxClocks = 8;
using ClockRegistry                = data_structures::Registry<ClockRegistryEntry, kMaxClocks>;

}  // namespace hardware

#endif  // ALKOS_KERNEL_INCLUDE_HARDWARE_CLOCK_INFRA_HPP_
