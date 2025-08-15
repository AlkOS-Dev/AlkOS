#include "drivers/tsc/tsc.hpp"
#include "modules/hardware.hpp"

// ------------------------------
// Clock callbacks
// ------------------------------

static u64 ReadCb(hardware::ClockRegistryEntry *) { return tsc::Read(); }

static u64 ReadNanoSecondsCb(hardware::ClockRegistryEntry *) { return 0; }

// ------------------------------
// Implementations
// ------------------------------

void tsc::Initialize()
{
    if (!IsAvailable()) {
        KernelTraceWarning("TSC is not available. Fallback to old technology...");
        return;
    }

    // NOTE: disabled RDTSC in user space
    SetUserSpaceAccess(false);

    TRACE_DEBUG("Detected TSC, current counter: %zu", Read());

    if (!IsStable()) {
        KernelTraceWarning("TSC is not stable. Fallback to old technology...");
    }

    // Clock information
    hardware::ClockRegistryEntry tsc_entry = {};
    tsc_entry.id                           = static_cast<u64>(arch::HardwareClockId::kTsc);
    // tsc_entry.frequency_kHz
    // tsc_entry.uncertainty_margin_per_sec
    tsc_entry.own_data = nullptr;  // No own data for TSC clock

    // Callbacks
    tsc_entry.read              = ReadCb;
    tsc_entry.read_nano_seconds = ReadNanoSecondsCb;
    tsc_entry.enable_device     = nullptr;  // No enable function
    tsc_entry.disable_device    = nullptr;  // No disable function
    tsc_entry.stop_counter      = nullptr;  // No stop function
    tsc_entry.resume_counter    = nullptr;  // No resume function

    // Register the TSC clock
    HardwareModule::Get().GetClockRegistry().Register(tsc_entry);
}
