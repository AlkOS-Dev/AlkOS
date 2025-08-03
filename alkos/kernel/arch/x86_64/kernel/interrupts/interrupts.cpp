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

void Interrupts::AllocateIoApic(const size_t num_apic)
{
    R_ASSERT_NOT_ZERO(num_apic, "No I/O APIC devices were found...");
    R_ASSERT_LT(
        num_apic, kTemporaryIoApicTableSize,
        "Number of Allocated apic devices overflows the table..."
    );
    ASSERT_ZERO(num_apic_, "I/O APIC devices should be initialized only once");

    TODO_WHEN_VMEM_WORKS
    num_apic_ = num_apic;
}

void Interrupts::InitializeIoApic(
    const size_t idx, const u8 id, const u32 address, const u32 gsi_base
)
{
    ASSERT_LT(idx, num_apic_, "Overflow detected on IO Apic table...");

    byte *ptr = mem_ + idx * sizeof(IoApic);
    new (reinterpret_cast<IoApic *>(ptr)) IoApic(id, address, gsi_base);
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

    const IoApic &io_apic = GetIoApicHandler(override->gsi);
    io_apic.ApplyOverrideRule(override);
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

    const IoApic &io_apic = GetIoApicHandler(nmi_source->gsi);
    io_apic.ApplyNmiRule(nmi_source);
}

IoApic &Interrupts::GetIoApicHandler(const u32 gsi)
{
    for (size_t idx = 0; idx < num_apic_; ++idx) {
        if (IoApic &io_apic = GetIoApic(idx); io_apic.IsInChargeOfGsi(gsi)) {
            return io_apic;
        }
    }

    R_FAIL_ALWAYS("No I/O APIC devices found handling given gsi...");
}
