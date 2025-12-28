#include <hal/impl/timers.hpp>

#include "data_structures/array_structures.hpp"
#include "modules/hardware.hpp"

// ------------------------------
// static functions
// ------------------------------

NODISCARD FAST_CALL bool PickClock(arch::HardwareClockId clock_id)
{
    if (HardwareModule::Get().GetClockRegistry().HasKey(clock_id)) {
        HardwareModule::Get().GetClockRegistry().SwitchSelected(clock_id);
        TRACE_INFO_INTERRUPTS(
            "Picked %s clock source as active",
            data_structures::IntegralToStringArray(static_cast<u64>(clock_id))
                .GetSafeStr()
                .GetCStr()
        );
        return true;
    }
    return false;
}

NODISCARD FAST_CALL bool PickEventClock(arch::HardwareEventClockId clock_id)
{
    if (HardwareModule::Get().GetEventClockRegistry().HasKey(clock_id)) {
        HardwareModule::Get().GetEventClockRegistry().SwitchSelected(clock_id);
        TRACE_INFO_INTERRUPTS(
            "Picked %s event clock source as active",
            data_structures::IntegralToStringArray(static_cast<u64>(clock_id))
                .GetSafeStr()
                .GetCStr()
        );
        return true;
    }
    return false;
}

// ------------------------------
// Implementations
// ------------------------------

namespace arch
{

void PickSystemClockSource()
{
    ASSERT_FALSE(HardwareModule::Get().GetClockRegistry().IsSelectedPicked());

    if (PickClock(HardwareClockId::kTsc)) {
        return;
    }

    if (PickClock(HardwareClockId::kHpet)) {
        return;
    }

    R_FAIL_ALWAYS("Support for available clocks is not implemented!");

    TODO_DEVICE_SUPPORT
    if (PickClock(HardwareClockId::kRtc)) {
        return;
    }

    TODO_DEVICE_SUPPORT
    if (PickClock(HardwareClockId::kPit)) {
        return;
    }

    TODO_DEVICE_SUPPORT
    if (PickClock(HardwareClockId::kInterruptBased)) {
        return;
    }

    R_FAIL_ALWAYS("Failed to pick system clock source, no clocks available!");
}

void PickSystemEventClockSource()
{
    ASSERT_FALSE(HardwareModule::Get().GetEventClockRegistry().IsSelectedPicked());

    if (PickEventClock(HardwareEventClockId::kLapic)) {
        HardwareModule::Get().GetInterrupts().GetIoApicTable() return;
    }

    R_FAIL_ALWAYS("Support for available event clocks is not implemented!");

    TODO_DEVICE_SUPPORT  // NO CORE LOCAl
        if (PickEventClock(HardwareEventClockId::kHpet))
    {
        return;
    }

    TODO_DEVICE_SUPPORT
    if (PickEventClock(HardwareEventClockId::kPit)) {
        return;
    }

    R_FAIL_ALWAYS("Failed to pick system clock source, no clocks available!");
}

}  // namespace arch
