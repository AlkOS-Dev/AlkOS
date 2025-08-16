#include "drivers/tsc/tsc.hpp"
#include "modules/hardware.hpp"

// ------------------------------
// Clock callbacks
// ------------------------------

static u64 ReadCb(hardware::ClockRegistryEntry *)
{
    const u64 tsc_value = tsc::Read();
    return tsc_value;
}

// ------------------------------
// Private functions
// ------------------------------

static void AlternativeTscCheck(hardware::ClockRegistryEntry &entry)
{
    FAIL_ALWAYS("Not implemented yet: Alternative TSC check");
}

static void PrepareTscInfo(hardware::ClockRegistryEntry &entry)
{
    u32 eax, ebx, ecx, unused;
    __get_cpuid(tsc::kIA32CpuidClockInfo, &eax, &ebx, &ecx, &unused);

    if (ebx == 0) {
        /* According to 19.17.4 of Intel SDM, EBX might be 0 */
        AlternativeTscCheck(entry);
    }

    const u64 denominator  = eax;
    const u64 numerator    = ebx;
    const u64 crystal_freq = ecx;

    TRACE_DEBUG(
        "denominator: %zu, numerator: %zu, crystal_freq: %zu", denominator, numerator, crystal_freq
    );

    const u64 tsc_freq_hz   = (numerator * crystal_freq) / denominator;
    entry.frequency_kHz     = tsc_freq_hz / 1000;  // Convert to kHz
    entry.clock_numerator   = kNanosInSecond;
    entry.clock_denominator = tsc_freq_hz;
    TODO_CLOCK_VALIDATION
    entry.ns_uncertainty_margin_per_sec = 0;  // Not known must be deduced in the future

    TRACE_DEBUG("TSC frequency: %zu", tsc_freq_hz);
}

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
    PrepareTscInfo(tsc_entry);
    tsc_entry.own_data = nullptr;  // No own data for TSC clock

    // Callbacks
    tsc_entry.read           = ReadCb;
    tsc_entry.enable_device  = nullptr;  // No enable function
    tsc_entry.disable_device = nullptr;  // No disable function
    tsc_entry.stop_counter   = nullptr;  // No stop function
    tsc_entry.resume_counter = nullptr;  // No resume function

    // Register the TSC clock
    HardwareModule::Get().GetClockRegistry().Register(tsc_entry);
}
