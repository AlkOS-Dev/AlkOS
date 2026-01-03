#include "modules/hardware.hpp"
#include "modules/memory.hpp"

#include "trace_framework.hpp"

internal::HardwareModule::HardwareModule() noexcept
{
    DEBUG_INFO_GENERAL("HardwareModule::HardwareModule()");

    Ps2Keyboard_.Init();
}

void internal::HardwareModule::RegisterPageFaultHandler()
{
    ::MemoryModule::Get().RegisterPageFault(::HardwareModule::Get());
}
