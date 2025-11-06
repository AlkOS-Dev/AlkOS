#include "modules/hardware.hpp"
// ------------------------------
// Assembly interface
// ------------------------------

extern "C" void HandleException(const u16 lirq, hal::ExceptionData *data)
{
    HardwareModule::Get().GetInterrupts().GetLit().HandleInterrupt(lirq, data);
}

extern "C" void HandleHardwareInterrupt(const u16 lirq)
{
    HardwareModule::Get()
        .GetInterrupts()
        .GetLit()
        .HandleInterrupt<intr::InterruptType::kHardwareInterrupt>(lirq);
}

extern "C" void HandleSoftwareInterrupt(const u16 lirq)
{
    HardwareModule::Get()
        .GetInterrupts()
        .GetLit()
        .HandleInterrupt<intr::InterruptType::kSoftwareInterrupt>(lirq);
}

// ------------------------------
// Implementations
// ------------------------------

namespace intr
{

}  // namespace intr
