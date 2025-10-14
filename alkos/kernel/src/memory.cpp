#include "modules/memory.hpp"

#include <extensions/debug.hpp>

internal::MemoryModule::MemoryModule() noexcept : Vmm_(Pmm_)
{
    TRACE_INFO("MemoryModule::MemoryModule()");
}
