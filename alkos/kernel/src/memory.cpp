#include "modules/memory.hpp"

#include <extensions/debug.hpp>

internal::MemoryModule::MemoryModule(const hal::KernelArguments &args) noexcept
    : VirtualMemoryManager_(args.page_table_phys_addr)
{
    TRACE_INFO("MemoryModule::MemoryModule()");
}
