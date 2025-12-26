#include "modules/hardware.hpp"
#include "modules/memory.hpp"

#include "trace_framework.hpp"

static void Ps2KeyboardHandler(intr::LitHwEntry &)
{
    HardwareModule::Get().GetPs2Keyboard().OnInterrupt();
}

internal::HardwareModule::HardwareModule() noexcept
{
    DEBUG_INFO_GENERAL("HardwareModule::HardwareModule()");

    Ps2Keyboard_.Init();
}

void internal::HardwareModule::RegisterInterruptHandlers()
{
    DEBUG_INFO_HARDWARE("Registering Ps2 Keyboard Handler");
    GetInterrupts().GetLit().InstallInterruptHandler<intr::InterruptType::kHardwareInterrupt>(
        1, intr::HwHandler{.handler = Ps2KeyboardHandler}
    );

    ::MemoryModule::Get().RegisterPageFault(::HardwareModule::Get());
}
