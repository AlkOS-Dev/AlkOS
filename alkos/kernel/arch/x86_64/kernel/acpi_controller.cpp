#include "acpi_controller.hpp"
#include "acpi/acpi_tables.hpp"
#include "modules/hardware.hpp"

#include <extensions/bit.hpp>
#include <extensions/debug.hpp>
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

NODISCARD size_t CountCores_(MadtTable &table)
{
    size_t cores{};
    table.ForEachTableEntry([&](const acpi_entry_hdr *entry) {
        if (entry->type != ACPI::MADTEntryTypeID<acpi_madt_lapic>::value) {
            return;
        }

        const auto table_ptr = reinterpret_cast<const acpi_madt_lapic *>(entry);

        if (!IsCoreUsable(table_ptr, cores)) {
            return;
        }

        ++cores;
    });

    return cores;
}

static void InitializeCores_(MadtTable &table)
{
    size_t cores{};
    table.ForEachTableEntry([&](const acpi_entry_hdr *entry) {
        const auto table_ptr = ACPI::TryToAccessTheTable<acpi_madt_lapic>(entry);

        if (!table_ptr) {
            return;
        }

        if (!IsBitEnabled<0>(table_ptr->flags)) {
            TRACE_INFO("Core with idx: %lu is not enabled...", cores);

            if (!IsBitEnabled<1>(table_ptr->flags)) {
                TRACE_WARNING("Core with idx: %lu is not online capable...", cores);
                return;
            }
        }

        HardwareModule::Get().GetCoresController().AllocateCore(
            cores++, static_cast<u64>(table_ptr->id), static_cast<u64>(table_ptr->uid)
        );
    });
}

static void PrepareIoApic_(MadtTable &table)
{
    table.ForEachTableEntry([&](const acpi_entry_hdr *entry) {
        const auto table_ptr = ACPI::TryToAccessTheTable<acpi_madt_ioapic>(entry);

        if (!table_ptr) {
            return;
        }

        TRACE_INFO(
            "Got IO APIC with id: %lu, at address: 0x%llX and base: 0x%llX", table_ptr->id,
            table_ptr->address, table_ptr->gsi_base
        );
    });
}

static void PrepareApicRules_(MadtTable &table)
{
    table.ForEachTableEntry([&](const acpi_entry_hdr *entry) {
        switch (entry->type) {
            case ACPI::MADTEntryTypeID<acpi_madt_lapic>::value:
            case ACPI::MADTEntryTypeID<acpi_madt_ioapic>::value:
                break;
            case ACPI::MADTEntryTypeID<acpi_madt_interrupt_source_override>::value: {
                const auto table_ptr =
                    reinterpret_cast<const acpi_madt_interrupt_source_override *>(entry);

                TRACE_INFO(
                    "Got I/O APIC Interrupt Source Override: "
                    "bus: %hhu, "
                    "source: %hhu, "
                    "gsi: %u, "
                    "flags: %04X",
                    table_ptr->bus, table_ptr->source, table_ptr->gsi, table_ptr->flags
                );
            } break;
            case ACPI::MADTEntryTypeID<acpi_madt_nmi_source>::value: {
                const auto table_ptr = reinterpret_cast<const acpi_madt_nmi_source *>(entry);

                TRACE_INFO(
                    "Got I/O APIC Non-maskable interrupt source: "
                    "gsi: %u, "
                    "flags: %04X",
                    table_ptr->gsi, table_ptr->flags
                );
            } break;
            case ACPI::MADTEntryTypeID<acpi_madt_lapic_nmi>::value: {
                const auto table_ptr = reinterpret_cast<const acpi_madt_lapic_nmi *>(entry);

                TRACE_INFO(
                    "Got Local APIC Non-maskable interrupts: "
                    "id: %hhu, "
                    "flags: %04X, "
                    "lint: %02X",
                    table_ptr->uid, table_ptr->flags, table_ptr->lint
                );
            } break;
            case ACPI::MADTEntryTypeID<acpi_madt_lapic_address_override>::value: {
                const auto table_ptr =
                    reinterpret_cast<const acpi_madt_lapic_address_override *>(entry);

                TRACE_INFO(
                    "Got Local APIC Address Override: "
                    "address: %16X",
                    table_ptr->address
                );
            } break;
            case ACPI::MADTEntryTypeID<acpi_madt_x2apic>::value:
                TRACE_INFO("x2apic not supported yet...");
                break;
            default:
                R_FAIL_ALWAYS("Found unsupported MADT table...");
                break;
        }
    });
}

// ------------------------------
// Implementations
// ------------------------------

void AcpiController::ParseTables()
{
    TRACE_INFO("Parsing ACPI Tables...");
    ParseMadt_();
}

void AcpiController::ParseMadt_()
{
    TRACE_INFO("Parsing MADT table...");

    auto table = ACPI::GetTable<acpi_madt>();
    R_ASSERT_TRUE(table.IsValid(), "MADT table is not found, only platform with apic supported...");

    /* Prepare cores */
    const size_t cores = CountCores_(table);
    TRACE_INFO("Found %d cores", cores);
    HardwareModule::Get().GetCoresController().AllocateCores(cores);

    /* Initialize core structures */
    InitializeCores_(table);

    /* Prepare interrupt data */
    PrepareIoApic_(table);

    /* Finally, process apic rules */
    PrepareApicRules_(table);
}
