#include "drivers/apic/local_apic.hpp"
#include "acpi/acpi.hpp"
#include "drivers/pic8259/pic8259.hpp"
#include "modules/hardware.hpp"

#include <assert.h>
#include <extensions/debug.hpp>
#include <todo.hpp>

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

    ASSERT_LT(nmi_source->lint, 2, "LINT number is out of range (0-1)");
    const u32 reg_offset =
        nmi_source->lint == 0 ? LocalApic::kLvtLint0RegRW : LocalApic::kLvtLint1RegRW;

    auto reg          = LocalApic::ReadRegister<LocalApic::LocalVectorTableRegister>(reg_offset);
    reg.delivery_mode = LocalApic::LocalVectorTableRegister::DeliveryMode::kNMI;
    LocalApic::WriteRegister(reg_offset, reg);
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

    TODO_WHEN_VMEM_WORKS
    /* Map local apic address to vmem */
    // TODO: currently: identity

    TRACE_INFO("Assuming APIC address as: %016X", local_apic_physical_address_);

    /* Enable Local Apic by ENABLE flag added to address (Might be enabled or might be not) */
    is_enabled_ = true;
    SetPhysicalAddressOnCore(local_apic_physical_address_);

    TRACE_INFO("Configuring LAPIC for core with id: %u", GetCoreId());

    /* Configure apic based on MADT entries */
    ParseMadtRules_();

    /* Set the Spurious Interrupt Vector Register bit 8 to start receiving interrupts */
    auto reg    = ReadRegister<SpuriousInterruptRegister>(kSpuriousInterruptRegRW);
    reg.enabled = true;
    reg.vector  = kSpuriousVector;

    WriteRegister(kSpuriousInterruptRegRW, reg);

    TRACE_INFO("Local APIC enabled...");
}
