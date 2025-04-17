#include "drivers/apic/local_apic.hpp"
#include "acpi/acpi.hpp"
#include "drivers/pic8259/pic8259.hpp"
#include "modules/hardware.hpp"

#include <assert.h>
#include <extensions/debug.hpp>
#include <todo.hpp>

using namespace LocalApic;

// ------------------------------
// Static functions
// ------------------------------

static void ApplyNmiSource_(const acpi_madt_lapic_nmi *nmi_source)
{
    ASSERT_NOT_NULL(nmi_source);

    TRACE_INFO(
        "Got LAPIC NMI source: "
        "flags: %hhu, "
        "lapic_id: %hhu, "
        "lint: %hhu",
        nmi_source->flags, nmi_source->uid, nmi_source->lint
    );
}

static void ParseMadtRules_()
{
    auto table = ACPI::GetTable<acpi_madt>();
    R_ASSERT_TRUE(table.IsValid(), "MADT table is not found, only platform with apic supported...");

    table.ForEachTableEntry([](const acpi_entry_hdr *entry) {
        const auto table_ptr = ACPI::TryToAccessTheTable<acpi_madt_lapic_nmi>(entry);

        if (table_ptr == nullptr) {
            return;
        }

        ApplyNmiSource_(table_ptr);
    });
}

// ------------------------------
// Implementations
// ------------------------------

void LocalApic::Enable()
{
    R_ASSERT_TRUE(IsSupported(), "APIC is not supported on this platform...");

    /* Disable PIC unit */
    Pic8259Disable();

    TODO_WHEN_VMEM_WORKS
    /* Map local apic address to vmem */
    // TODO: currently: identity

    const u64 lapic_address = HardwareModule::Get().GetInterrupts().GetLocalApicPhysicalAddress();
    TRACE_INFO("Assuming APIC address as: %016X", lapic_address);

    /* Enable Local Apic by ENABLE flag added to address (Might be enabled or might be not) */
    SetPhysicalAddress(lapic_address);

    TRACE_INFO("Configuring LAPIC for core with id: %u", GetCoreId());

    /* Configure apic based on MADT entries */
    ParseMadtRules_();

    /* Set the Spurious Interrupt Vector Register bit 8 to start receiving interrupts */
    auto reg    = CastRegister<SpuriousInterruptRegister>(ReadRegister(kSpuriousInterruptRegRW));
    reg.enabled = 1;

    WriteRegister(kSpuriousInterruptRegRW, ToRawRegister(reg));

    TRACE_INFO("Local APIC enabled...");
}
