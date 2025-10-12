#ifndef ALKOS_KERNEL_INCLUDE_HARDWARE_CLOCK_INFRA_HPP_
#define ALKOS_KERNEL_INCLUDE_HARDWARE_CLOCK_INFRA_HPP_

#include <extensions/data_structures/data_structures.hpp>
#include <hal/constants.hpp>

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
        const u64 timer_val = GetSelected().read(&GetSelected());
        TRACE_DEBUG("Timer value read: %zu", timer_val);
        TRACE_DEBUG(
            "Numerator: %zu, Denominator: %zu", GetSelected().clock_numerator,
            GetSelected().clock_denominator
        );

        const __uint128_t intermediate_value =
            static_cast<__uint128_t>(timer_val) * GetSelected().clock_numerator;
        const u64 time_ns = static_cast<u64>(intermediate_value / GetSelected().clock_denominator);

        return time_ns;
    }
};

}  // namespace hardware

#endif  // ALKOS_KERNEL_INCLUDE_HARDWARE_CLOCK_INFRA_HPP_
