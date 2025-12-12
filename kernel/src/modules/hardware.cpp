#include "modules/hardware.hpp"

#include "trace_framework.hpp"

static void Ps2KeyboardHandler(intr::LitHwEntry &)
{
    HardwareModule::Get().GetPs2Keyboard().OnInterrupt();
}

internal::HardwareModule::HardwareModule() noexcept
{
    DEBUG_INFO_GENERAL("HardwareModule::HardwareModule()");

    // Initialize the generic PS/2 driver
    Ps2Keyboard_.Init();

    // Map Logical IRQ 1 (Standard Keyboard IRQ) to the driver
    // Note: Interrupts::Init() must enable the hardware controller (IOAPIC/PIC)
    GetInterrupts().GetLit().InstallInterruptHandler<intr::InterruptType::kHardwareInterrupt>(
        1, intr::HwHandler{.handler = Ps2KeyboardHandler}
    );
}
