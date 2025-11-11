#include <hal/impl/interrupts.hpp>

#include "cpu/utils.hpp"
#include "drivers/apic/local_apic.hpp"
#include "drivers/pic8259/pic8259.hpp"
#include "drivers/tsc/tsc.hpp"
#include "interrupts/idt.hpp"
#include "trace_framework.hpp"

#include <modules/hardware.hpp>
#include "trace_framework.hpp"

using namespace arch;

void Interrupts::Init()
{
    DEBUG_INFO_INTERRUPTS("Initialising interrupts system...");

    BlockHardwareInterrupts();

    /* Disable PIC unit */
    Pic8259Disable();

    ReplacePicDriverWithLapic_();

    /* Enable IO apics */
    for (IoApic &io_apic : GetIoApicTable()) {
        io_apic.PrepareDefaultConfig();
    }

    /* Replace first stage PIC with new APIC chip on startup Core */
    local_apic_.Enable();

    tsc::Initialize();

    EnableHardwareInterrupts();

    DEBUG_INFO_INTERRUPTS("Interrupts system initialised...");
}

void Interrupts::FirstStageInit()
{
    DEBUG_INFO_INTERRUPTS("Interrupts first stage init...");

    InitPic8259(kIrq1Offset, kIrq2Offset);
    MapToLogicalInterrupts_();
    SetupPicAsDefaultDriver_();
    trace::AdvanceTracingStage();
    InitializeDefaultIdt_();
}

void Interrupts::ApplyIoApicOverride(const acpi_madt_interrupt_source_override *override)
{
    ASSERT_NOT_NULL(override);

    DEBUG_INFO_INTERRUPTS(
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

    DEBUG_INFO_INTERRUPTS(
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

void Interrupts::MapToLogicalInterrupts_()
{
    // REFER to layout established by idt.nasm file

    // Map exceptions
    for (u16 idt_idx = 0; idt_idx < kNumX86_64CpuExceptions; idt_idx++) {
        HardwareModule::Get()
            .GetInterrupts()
            .GetLit()
            .MapLogicalInterruptToHw<intr::InterruptType::kException>(idt_idx, idt_idx);

        HardwareModule::Get()
            .GetInterrupts()
            .GetLit()
            .InstallInterruptHandler<intr::InterruptType::kException>(
                idt_idx, intr::ExcHandler{.handler = DefaultExceptionHandler}
            );
    }

    // Map basic pic or lapic irqs
    for (u16 idx = 0; idx < kNumX86_64Irqs; idx++) {
        HardwareModule::Get()
            .GetInterrupts()
            .GetLit()
            .MapLogicalInterruptToHw<intr::InterruptType::kHardwareInterrupt>(
                idx, idx + kNumX86_64CpuExceptions
            );

        HardwareModule::Get()
            .GetInterrupts()
            .GetLit()
            .InstallInterruptHandler<intr::InterruptType::kHardwareInterrupt>(
                idx, intr::HwHandler{.handler = SimpleIrqHandler}
            );
    }

    // Map timer handler
    HardwareModule::Get()
        .GetInterrupts()
        .GetLit()
        .InstallInterruptHandler<intr::InterruptType::kHardwareInterrupt>(
            0, intr::HwHandler{.handler = TimerIsr}
        );

    // Map test software irq
    HardwareModule::Get()
        .GetInterrupts()
        .GetLit()
        .MapLogicalInterruptToHw<intr::InterruptType::kSoftwareInterrupt>(0, 48);

    HardwareModule::Get()
        .GetInterrupts()
        .GetLit()
        .InstallInterruptHandler<intr::InterruptType::kSoftwareInterrupt>(
            0, intr::SwHandler{.handler = TestIsr}
        );
}

void Interrupts::SetupPicAsDefaultDriver_()
{
    for (u16 idx = 0; idx < kNumX86_64Irqs; idx++) {
        HardwareModule::Get().GetInterrupts().GetLit().InstallInterruptDriver(
            idx, &Pic8259InterruptDriver()
        );
    }
}

void Interrupts::ReplacePicDriverWithLapic_()
{
    for (u16 idx = 0; idx < kNumX86_64Irqs; idx++) {
        HardwareModule::Get().GetInterrupts().GetLit().InstallInterruptDriver(
            idx, &local_apic_.GetInterruptDriver()
        );
    }
}
