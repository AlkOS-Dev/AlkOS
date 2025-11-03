#include "drivers/apic/io_apic.hpp"
#include "drivers/apic/local_apic.hpp"

#include <extensions/bit.hpp>
#include <extensions/debug.hpp>
#include <todo.hpp>
#include "interrupts/idt.hpp"

// ------------------------------
// static functions
// ------------------------------

FAST_CALL void ApplyAcpiFlags(const u16 flags, IoApic::LowerTableRegister &reg)
{
    if (AreBitsEnabled(flags, static_cast<u16>(ACPI_MADT_POLARITY_ACTIVE_HIGH))) {
        reg.pin_polarity = IoApic::LowerTableRegister::PinPolarity::kActiveHigh;
    } else if (AreBitsEnabled(flags, static_cast<u16>(ACPI_MADT_POLARITY_ACTIVE_LOW))) {
        reg.pin_polarity = IoApic::LowerTableRegister::PinPolarity::kActiveLow;
    }

    if (AreBitsEnabled(flags, static_cast<u16>(ACPI_MADT_TRIGGERING_EDGE))) {
        reg.trigger_mode = IoApic::LowerTableRegister::TriggerMode::kEdge;
    } else if (AreBitsEnabled(flags, static_cast<u16>(ACPI_MADT_TRIGGERING_LEVEL))) {
        reg.trigger_mode = IoApic::LowerTableRegister::TriggerMode::kLevel;
    }
}

// ------------------------------
// Implementations
// ------------------------------

IoApic::IoApic(const u8 id, const u32 address, const u32 gsi_base)
    : physical_address_(address), gsi_base_(gsi_base), id_(id)
{
    ASSERT_NOT_ZERO(address);

    TODO_WHEN_VMEM_WORKS
    /* TODO: Map the address first, currently identity */
    virtual_address_ = physical_address_;

    version_ = static_cast<u8>(ReadRegister(kIoApicVerReg)); /* Removes upper bits */
    num_entries_ =
        static_cast<u8>(ReadRegister(kIoApicVerReg) >> 16) + 1; /* Access bits [16, 23] */

    TRACE_INFO(
        "Got IO APIC (%hhu) "
        "at address: 0x%llX, "
        "version: %hhu, "
        "number of entries: %hhu, "
        "base: 0x%X",
        id_, virtual_address_, version_, num_entries_, gsi_base_
    );
}

void IoApic::PrepareDefaultConfig() const
{
    for (u32 idx = 0; idx < num_entries_; ++idx) {
        auto reg_low = ReadLowerTableRegister(idx);

        /* Note: vector not initialized */
        reg_low.delivery_mode    = LowerTableRegister::DeliveryMode::kFixed;
        reg_low.destination_mode = LowerTableRegister::DestinationMode::kPhysical;
        reg_low.pin_polarity     = LowerTableRegister::PinPolarity::kActiveHigh;
        reg_low.trigger_mode     = LowerTableRegister::TriggerMode::kEdge;
        reg_low.mask             = LowerTableRegister::EnabledFlag::kEnabled;

        WriteLowerTableRegister(idx, reg_low);
    }
}

void IoApic::ApplyOverrideRule(const acpi_madt_interrupt_source_override *override) const
{
    ASSERT_NOT_NULL(override);

    auto reg_low = ReadLowerTableRegister(override->gsi - GetGsiBase());

    /* Apply flags */
    ApplyAcpiFlags(override->flags, reg_low);

    /* Apply redirection */
    reg_low.vector = kIrq1Offset + override->source;

    /* Write back */
    WriteLowerTableRegister(override->gsi - GetGsiBase(), reg_low);
}

void IoApic::ApplyNmiRule(const acpi_madt_nmi_source *nmi_source) const
{
    ASSERT_NOT_NULL(nmi_source);

    auto reg_low = ReadLowerTableRegister(nmi_source->gsi - GetGsiBase());

    /* Apply flags */
    ApplyAcpiFlags(nmi_source->flags, reg_low);
    reg_low.delivery_mode = LowerTableRegister::DeliveryMode::kNMI;

    /* Write back */
    WriteLowerTableRegister(nmi_source->gsi - GetGsiBase(), reg_low);
}
