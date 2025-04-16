#include "core.hpp"

#include <extensions/debug.hpp>

using namespace arch;

Core::Core(const u64 acpi_id, const u64 apic_id) : acpi_id_(acpi_id), apic_id_(apic_id)
{
    TRACE_INFO("Core with ACPI ID: %lu, APIC ID: %lu created", acpi_id, apic_id);
}

void Core::EnableCore()
{
    // TODO:
}
