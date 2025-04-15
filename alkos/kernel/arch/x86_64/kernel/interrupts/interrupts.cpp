#include "interrupts.hpp"

#include <acpi/acpi_tables.hpp>

#include "drivers/pic8259/pic8259.hpp"
#include "interrupts/idt.hpp"

#include <extensions/debug.hpp>

using namespace arch;

void Interrupts::Initialise()
{
    TRACE_INFO("Initialising interrupts system...");

    auto table = ACPI::GetTable<acpi_madt>();
    R_ASSERT_TRUE(table.IsValid(), "MADT table is not found");
    ` qqqqqqqqqqqqqqqq1111111111111111
}

void Interrupts::FirstStageInit()
{
    TRACE_INFO("Interrupts first stage init...");

    InitPic8259(kIrq1Offset, kIrq2Offset);
    InitializeDefaultIdt_();
}
