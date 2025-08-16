#ifndef ALKOS_KERNEL_INCLUDE_HARDWARE_CLOCK_INFRA_HPP_
#define ALKOS_KERNEL_INCLUDE_HARDWARE_CLOCK_INFRA_HPP_

#include <constants.hpp>
#include <extensions/data_structures/data_structures.hpp>

namespace hardware
{

struct ClockRegistryEntry : data_structures::RegistryEntry {
    /* Clock numbers */
    u64 frequency_kHz;                  // Frequency in kHz
    u64 ns_uncertainty_margin_per_sec;  // Uncertainty margin in femto seconds per second
    u64 clock_numerator;                // For conversion to nanoseconds, this is the numerator
    u64 clock_denominator;              // For conversion to nanoseconds, this is the denominator

    /* Callbacks */
    u64 (*read)(ClockRegistryEntry* clock_entry);
    bool (*enable_device)(ClockRegistryEntry* clock_entry);
    bool (*disable_device)(ClockRegistryEntry* clock_entry);
    void (*stop_counter)(ClockRegistryEntry* clock_entry);
    void (*resume_counter)(ClockRegistryEntry* clock_entry);

    /* Own data */
    void* own_data;
};

static constexpr size_t kMaxClocks = 8;
class ClockRegistry : public data_structures::Registry<ClockRegistryEntry, kMaxClocks>
{
    public:
    /* Not safe for concurrent access */
    NODISCARD FORCE_INLINE_F u64 ReadTimeNsUnsafe()
    {
        TODO_WHEN_TIMER_INFRA_DONE
        // TODO: Improve calculation robustness on overflows

        const u64 timer_val = GetActive().read(&GetActive());
        return (timer_val * GetActive().clock_numerator) / GetActive().clock_denominator;
    }
};

}  // namespace hardware

#endif  // ALKOS_KERNEL_INCLUDE_HARDWARE_CLOCK_INFRA_HPP_
