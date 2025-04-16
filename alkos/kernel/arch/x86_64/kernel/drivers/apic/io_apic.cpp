#include "drivers/apic/io_apic.hpp"

#include <extensions/bit.hpp>
#include <extensions/debug.hpp>
#include <todo.hpp>

IoApic::IoApic(const u8 id, const u32 address, const u32 gsi_base)
    : physical_address_(address), gsi_base_(gsi_base), id_(id)
{
    TODO_WHEN_VMEM_WORKS
    /* TODO: Map the address first, currently identity */
    virtual_address_ = physical_address_;

    version_ = static_cast<u8>(ReadRegister(kIoApicVerReg)); /* Removes upper bits */
    num_entries_ =
        static_cast<u8>(ReadRegister(kIoApicVerReg) >> 16) + 1; /* Access bits [16, 23] */

    TRACE_INFO(
        "Got IO APIC (%lu) "
        "at address: 0x%llX, "
        "version: %hhu, "
        "number of entries: %hhu, "
        "base: 0x%llX",
        id_, virtual_address_, version_, num_entries_, gsi_base_
    );

    PrepareDefaultConfig();
}

void IoApic::PrepareDefaultConfig() const
{
    for (u32 idx = gsi_base_; idx < gsi_base_ + num_entries_; ++idx) {
        const u32 reg_raw          = ReadRegister(IoApicTableReg(idx));
        LowerTableRegister reg_low = *reinterpret_cast<const LowerTableRegister *>(&reg_raw);

        reg_low.delivery_mode    = static_cast<u32>(DeliveryMode::kFixed);
        reg_low.destination_mode = static_cast<u32>(DestinationMode::kPhysical);
        reg_low.pin_polarity     = static_cast<u32>(PinPolarity::kActiveHigh);
        reg_low.trigger_mode     = static_cast<u32>(TriggerMode::kEdge);
        reg_low.mask             = static_cast<u32>(Mask::kEnabled);

        WriteRegister(IoApicTableReg(idx), *reinterpret_cast<u32 *>(&reg_low));
    }
}

void IoApic::ApplyOverrideRule(const acpi_madt_interrupt_source_override *override)
{
    ASSERT_NOT_NULL(override);

    const u32 reg_idx          = IoApicTableReg(override->gsi - GetGsiBase());
    const u32 reg_raw          = ReadRegister(reg_idx);
    LowerTableRegister reg_low = *reinterpret_cast<const LowerTableRegister *>(&reg_raw);

    /* Apply flags */
    if (AreBitsEnabled(override->flags, static_cast<u16>(ACPI_MADT_POLARITY_ACTIVE_HIGH))) {
        reg_low.pin_polarity = static_cast<u32>(PinPolarity::kActiveHigh);
    } else if (AreBitsEnabled(override->flags, static_cast<u16>(ACPI_MADT_POLARITY_ACTIVE_LOW))) {
        reg_low.pin_polarity = static_cast<u32>(PinPolarity::kActiveLow);
    }

    if (AreBitsEnabled(override->flags, static_cast<u16>(ACPI_MADT_TRIGGERING_EDGE))) {
        reg_low.trigger_mode = static_cast<u32>(TriggerMode::kEdge);
    } else if (AreBitsEnabled(override->flags, static_cast<u16>(ACPI_MADT_TRIGGERING_LEVEL))) {
        reg_low.trigger_mode = static_cast<u32>(TriggerMode::kLevel);
    }

    /* Apply redirection */
    // TODO:
    reg_low.vector = override->source;

    /* Write back */
    WriteRegister(reg_idx, *reinterpret_cast<u32 *>(&reg_low));
}
