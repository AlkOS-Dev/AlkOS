#include <hal/impl/acpi_controller.hpp>

#include <acpi/acpi_tables.hpp>
#include <modules/hardware.hpp>

#include <extensions/bit.hpp>
#include <extensions/debug.hpp>
#include <trace.hpp>

#include "drivers/hpet/hpet.hpp"
#include "hardware/cores.hpp"

using namespace arch;

// ------------------------------
// Static functions
// ------------------------------

using MadtTable = ACPI::Table<acpi_madt>;
NODISCARD static bool IsCoreUsable(const acpi_madt_lapic *table)
{
    if (!IsBitEnabled<0>(table->flags)) {
        TRACE_INFO("Core with id: %hhu is not enabled...", table->id);

        if (!IsBitEnabled<1>(table->flags)) {
            TRACE_WARNING("Core with id: %hhu is not online capable...", table->id);
            return false;
        }
    }

    return true;
}

static void InitializeCores_(MadtTable &table)
{
    /* Initialize core structures */
    u32 cores_to_use = 0;
    u32 total_cores  = 0;
    u32 max_hw_id    = 0;
    table.ForEachTableEntry([&](const acpi_entry_hdr *entry) {
        const auto table_ptr = ACPI::TryToAccessTheTable<acpi_madt_lapic>(entry);

        if (!table_ptr) {
            return;
        }

        if (!IsCoreUsable(table_ptr)) {
            return;
        }

        total_cores++;
        if (cores_to_use >= kMaxCores) {
            return;
        }

        cores_to_use++;
        max_hw_id = std::max(max_hw_id, static_cast<u32>(table_ptr->id));
    });

    TRACE_INFO("Found %u cores, using: %u", total_cores, cores_to_use);

    HardwareModule::Get().GetCoresController().AllocateTables(cores_to_use, max_hw_id);

    hardware::CoreMask mask{};
    u16 counter = 0;
    table.ForEachTableEntry([&](const acpi_entry_hdr *entry) {
        const auto table_ptr = ACPI::TryToAccessTheTable<acpi_madt_lapic>(entry);

        if (!table_ptr) {
            return;
        }

        if (!IsCoreUsable(table_ptr)) {
            return;
        }

        hardware::CoreConfig config{};
        ASSERT_LE(static_cast<size_t>(table_ptr->id), kBitMask16);
        config.hwid = static_cast<u16>(table_ptr->id);
        ASSERT_LE(static_cast<size_t>(table_ptr->uid), kBitMask16);
        config.acpi_id = static_cast<u16>(table_ptr->uid);
        config.lid     = counter;
        config.enabled = IsBitEnabled<0>(table_ptr->flags);

        HardwareModule::Get().GetCoresController().AllocateCore(config);
        mask.SetTrue(counter++);
    });
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
            "address: %016llX, "
            "rsvd: %hu",
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
