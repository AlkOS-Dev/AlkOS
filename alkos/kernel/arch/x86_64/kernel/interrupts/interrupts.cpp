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
    R_ASSERT_TRUE(table.IsValid(), "MADT table is not found, only platform with apic supported...");

    auto lapic_address =
        reinterpret_cast<void *>(table.GetNative()->local_interrupt_controller_address);

    table.ForEachTableEntry([](acpi_entry_hdr *entry) {
        TRACE_INFO("Table with size: %lu discovered and type: %u", entry->length, entry->type);
    });
}

void Interrupts::FirstStageInit()
{
    TRACE_INFO("Interrupts first stage init...");

    InitPic8259(kIrq1Offset, kIrq2Offset);
    InitializeDefaultIdt_();
}
