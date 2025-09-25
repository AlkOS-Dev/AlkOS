#include "hal/timers.hpp"
#include "extensions/data_structures/array_structures.hpp"
#include "modules/hardware.hpp"
#include "trace.hpp"

// ------------------------------
// static functions
// ------------------------------

NODISCARD FAST_CALL bool PickClock(arch::HardwareClockId clock_id)
{
    if (HardwareModule::Get().GetClockRegistry().HasKey(clock_id)) {
        HardwareModule::Get().GetClockRegistry().SwitchSelected(clock_id);
        KernelTraceSuccess(
            "Picked %s clock source as active",
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

}  // namespace arch
