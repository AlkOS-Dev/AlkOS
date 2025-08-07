#include "acpi_controller.hpp"
#include "acpi/acpi_tables.hpp"
#include "drivers/hpet/hpet.hpp"
#include "modules/hardware.hpp"

#include <extensions/bit.hpp>
#include <extensions/debug.hpp>
#include <trace.hpp>

using namespace arch;

// ------------------------------
// Static functions
// ------------------------------

using MadtTable = ACPI::Table<acpi_madt>;
NODISCARD static bool IsCoreUsable(const acpi_madt_lapic *table, const size_t core_idx)
{
    if (!IsBitEnabled<0>(table->flags)) {
        TRACE_INFO("Core with idx: %lu is not enabled...", core_idx);

        if (!IsBitEnabled<1>(table->flags)) {
            TRACE_WARNING("Core with idx: %lu is not online capable...", core_idx);
            return false;
        }
    }

    return true;
}

static void InitializeCores_(MadtTable &table)
{
    /* Initialize core structures */
    size_t cores{};
    table.ForEachTableEntry([&](const acpi_entry_hdr *entry) {
        const auto table_ptr = ACPI::TryToAccessTheTable<acpi_madt_lapic>(entry);

        if (!table_ptr) {
            return;
        }

        if (!IsCoreUsable(table_ptr, cores++)) {
            return;
        }

        HardwareModule::Get().GetCoresController().GetCoreTable().PushEmplace(
            static_cast<u64>(table_ptr->id), static_cast<u64>(table_ptr->uid)
        );
    });

    TRACE_INFO("Found %zu cores", cores);
}

static void PrepareIoApic_(MadtTable &table)
{
    size_t num_apic{};
    table.ForEachTableEntry([&](const acpi_entry_hdr *entry) {
        num_apic += (ACPI::TryToAccessTheTable<acpi_madt_ioapic>(entry) != nullptr);
    });
    TRACE_INFO("Detected %lu I/O APIC devices...", num_apic);

    table.ForEachTableEntry([](const acpi_entry_hdr *entry) {
        const auto table_ptr = ACPI::TryToAccessTheTable<acpi_madt_ioapic>(entry);

        if (!table_ptr) {
            return;
        }

        HardwareModule::Get().GetInterrupts().GetIoApicTable().PushEmplace(
            static_cast<u8>(table_ptr->id), static_cast<u32>(table_ptr->address),
            static_cast<u32>(table_ptr->gsi_base)
        );
    });
}

static void PrepareApicRules_(MadtTable &table)
{
    table.ForEachTableEntry([](const acpi_entry_hdr *entry) {
        switch (entry->type) {
            case ACPI::MADTEntryTypeID<acpi_madt_lapic>::value:
            case ACPI::MADTEntryTypeID<acpi_madt_ioapic>::value:
            case ACPI::MADTEntryTypeID<acpi_madt_lapic_nmi>::value:
            case ACPI::MADTEntryTypeID<acpi_madt_lapic_address_override>::value:
                break;
            case ACPI::MADTEntryTypeID<acpi_madt_interrupt_source_override>::value:
                HardwareModule::Get().GetInterrupts().ApplyIoApicOverride(
                    reinterpret_cast<const acpi_madt_interrupt_source_override *>(entry)
                );
                break;
            case ACPI::MADTEntryTypeID<acpi_madt_nmi_source>::value:
                HardwareModule::Get().GetInterrupts().ApplyIoApicNmi(
                    reinterpret_cast<const acpi_madt_nmi_source *>(entry)
                );
                break;
            case ACPI::MADTEntryTypeID<acpi_madt_x2apic>::value:
                TRACE_INFO("x2apic not supported yet...");
                break;
            default:
                KernelTraceWarning("Found unsupported MADT table. Skipping...");
        }
    });
}

static void ParseLApicAddress_(MadtTable &table)
{
    TRACE_INFO(
        "Local APIC physical address: %08X", table.GetNative()->local_interrupt_controller_address
    );

    HardwareModule::Get().GetInterrupts().GetLocalApic().SetPhysicalAddress(
        table.GetNative()->local_interrupt_controller_address
    );

    table.ForEachTableEntry([](const acpi_entry_hdr *entry) {
        const auto table_ptr = ACPI::TryToAccessTheTable<acpi_madt_lapic_address_override>(entry);

        if (table_ptr == nullptr) {
            return;
        }

        TRACE_INFO(
            "Found LAPIC Address Override: "
            "address: %016X, "
            "rsvd: %hhu",
            table_ptr->address, table_ptr->rsvd
        );

        HardwareModule::Get().GetInterrupts().GetLocalApic().SetPhysicalAddress(table_ptr->address);
    });
}

// ------------------------------
// Implementations
// ------------------------------

void AcpiController::ParseTables()
{
    TRACE_INFO("Parsing ACPI Tables...");
    ParseMadt_();
    ParseHpet_();
}

void AcpiController::ParseMadt_()
{
    TRACE_INFO("Parsing MADT table...");

    auto table = ACPI::GetTable<acpi_madt>();
    R_ASSERT_TRUE(table.IsValid(), "MADT table is not found, only platform with apic supported...");

    /* Prepare LAPIC */
    ParseLApicAddress_(table);

    /* Initialize core structures */
    InitializeCores_(table);

    /* Prepare interrupt data */
    PrepareIoApic_(table);

    /* Finally, process apic rules */
    PrepareApicRules_(table);
}

void AcpiController::ParseHpet_()
{
    TRACE_INFO("Parsing HPET table...");
    const auto table = ACPI::GetTable<acpi_hpet>();

    if (!table.IsValid()) {
        TRACE_INFO("HPET table not found...");
        return;
    }

    TODO_WHEN_TIMER_INFRA_DONE
    /* TODO: Use the driver */

    ASSERT_FALSE(
        static_cast<bool>(HardwareModule::Get().GetInterrupts().GetHpet()),
        "HPET already initialized!"
    );
    HardwareModule::Get().GetInterrupts().GetHpet().emplace(table.GetNative());
}
