#ifndef ALKOS_KERNEL_INCLUDE_HARDWARE_EVENT_CLOCK_INFRA_HPP_
#define ALKOS_KERNEL_INCLUDE_HARDWARE_EVENT_CLOCK_INFRA_HPP_

#include <time.h>
#include <extensions/data_structures/bit_array.hpp>
#include <extensions/data_structures/hash_maps.hpp>
#include "hardware/core_mask.hpp"

namespace hardware
{
enum class EventClockFeatures : u8 {
    kIsCoreLocal = 0,  // Clock is local to the core
    kLast,
};

enum class EventClockState : u8 {
    kDisabled = 0,  // Clock is disabled
    kPeriodic,      // Clock is in periodic mode
    kOneshot,       // Clock is in oneshot mode
    kOneshotIdle,   // Clock is in oneshot mode but no event is scheduled
    klast,
};

struct alignas(arch::kCacheLineSizeBytes) EventClockRegistryEntry : data_structures::RegistryEntry {
    /* Clock numbers */
    u64 min_next_event_time_ns;  // Minimum time for the next event in nanoseconds
    u64 max_event_time_ns;       // Maximum time for the event in nanoseconds

    /* Clock specific data */
    data_structures::BitArray<32> features;  // Features of the event clock, e.g., core-local
    CoreMask supported_cores;                // Cores that support this event clock

    /* infra data */
    u64 next_event_time_ns;  // Time for the next event in nanoseconds
    EventClockState state;   // Current state of the clock

    /* Driver data */
    void *own_data;  // Pointer to the clock's own data, used for callback

    /* callbacks */
    struct callbacks {
        TODO_WHEN_TIMER_INFRA_DONE
        // TODO: suspend/resume for os supsend etc

        u32 (*next_event)(EventClockRegistryEntry *, u64);  // Callback to set next event time
        u32 (*set_oneshot)(EventClockRegistryEntry *);      // Callback to set clock state
        u32 (*set_periodic)(EventClockRegistryEntry *);     // Callback to set clock state
        u32 (*handler)(
            EventClockRegistryEntry *
        );  // Optional function to perform some action on call
    } cbs;
};

static constexpr size_t kMaxEventClocks = 8;
class EventClockRegistry
    : public data_structures::Registry<EventClockRegistryEntry, kMaxEventClocks>
{
};
}  // namespace hardware

#endif  // ALKOS_KERNEL_INCLUDE_HARDWARE_EVENT_CLOCK_INFRA_HPP_
