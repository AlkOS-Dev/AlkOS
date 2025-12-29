#ifndef KERNEL_SRC_HARDWARE_EVENT_CLOCK_INFRA_HPP_
#define KERNEL_SRC_HARDWARE_EVENT_CLOCK_INFRA_HPP_

#include <time.h>
#include <data_structures/bit_array.hpp>
#include <data_structures/hash_maps.hpp>
#include "hardware/core_mask.hpp"

namespace hardware
{

struct PACK EventClockFlags {
    bool IsCoreLocal : 1;
    u32 padding : 31;
};
static_assert(sizeof(EventClockFlags) == sizeof(u32));

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

    /* Clock specific data */
    EventClockFlags flags;     // Features of the event clock, e.g., core-local
    CoreMask supported_cores;  // Cores that support this event clock

    /* infra data */
    u64 next_event_time_ns;  // Time for the next event in nanoseconds
    EventClockState state;   // Current state of the clock

    TODO_WHEN_TIMER_INFRA_DONE
    // TODO: suspend/resume for os supsend etc

    /* Driver data */
    void *own_data;  // Pointer to the clock's own data, used for callback

    /* callbacks */
    struct callbacks {
        u32 (*next_event)(EventClockRegistryEntry *, u64);  // Callback to set next event time
        u32 (*set_oneshot)(EventClockRegistryEntry *);      // Callback to set clock state
        u32 (*set_periodic)(EventClockRegistryEntry *);     // Callback to set clock state
        void (*on_entry)(EventClockRegistryEntry *);        // optional
        void (*on_exit)(EventClockRegistryEntry *);         // optional
    } cbs;
};

static constexpr size_t kMaxEventClocks = 8;
class EventClockRegistry
    : public data_structures::Registry<EventClockRegistryEntry, kMaxEventClocks>
{
};
}  // namespace hardware

#endif  // KERNEL_SRC_HARDWARE_EVENT_CLOCK_INFRA_HPP_
