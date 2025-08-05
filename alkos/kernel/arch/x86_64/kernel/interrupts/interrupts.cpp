#include "interrupts.hpp"

#include "drivers/apic/local_apic.hpp"
#include "drivers/pic8259/pic8259.hpp"
#include "interrupts/idt.hpp"

#include <extensions/debug.hpp>
#include <extensions/new.hpp>

using namespace arch;

void Interrupts::Initialise()
{
    TRACE_INFO("Initialising interrupts system...");

    /* Replace first stage PIC with new APIC chip on startup Core */
    LocalApic::Enable();
    is_apic_initialized_ = true;

    TRACE_INFO("Interrupts system initialised...");
}

void Interrupts::FirstStageInit()
{
    TRACE_INFO("Interrupts first stage init...");

    InitPic8259(kIrq1Offset, kIrq2Offset);
    InitializeDefaultIdt_();
}

void Interrupts::ApplyIoApicOverride(const acpi_madt_interrupt_source_override *override)
{
    ASSERT_NOT_NULL(override);

    TRACE_INFO(
        "Got I/O APIC Interrupt Source Override: "
        "bus: %hhu, "
        "source: %hhu, "
        "gsi: %u, "
        "flags: %04X",
        override->bus, override->source, override->gsi, override->flags
    );

    GetIoApicHandler(override->gsi).ApplyOverrideRule(override);
}

void Interrupts::ApplyIoApicNmi(const acpi_madt_nmi_source *nmi_source)
{
    ASSERT_NOT_NULL(nmi_source);

    TRACE_INFO(
        "Got I/O APIC Non-maskable interrupt source: "
        "gsi: %u, "
        "flags: %04X",
        nmi_source->gsi, nmi_source->flags
    );

    GetIoApicHandler(nmi_source->gsi).ApplyNmiRule(nmi_source);
}

IoApic &Interrupts::GetIoApicHandler(const u32 gsi)
{
    for (IoApic &io_apic : GetIoApicTable()) {
        if (io_apic.IsInChargeOfGsi(gsi)) {
            return io_apic;
        }
    }

    R_FAIL_ALWAYS("No I/O APIC devices found handling given gsi...");
}
