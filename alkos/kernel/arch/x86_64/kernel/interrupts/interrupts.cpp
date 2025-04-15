#include "interrupts.hpp"

#include "drivers/pic8259/pic8259.hpp"
#include "interrupts/idt.hpp"

#include <extensions/debug.hpp>
#include <extensions/new.hpp>

using namespace arch;

void Interrupts::Initialise() { TRACE_INFO("Initialising interrupts system..."); }

void Interrupts::FirstStageInit()
{
    TRACE_INFO("Interrupts first stage init...");

    InitPic8259(kIrq1Offset, kIrq2Offset);
    InitializeDefaultIdt_();
}

void Interrupts::AllocateIoApic(const size_t num_apic)
{
    R_ASSERT_NOT_ZERO(num_apic, "No I/O APIC devices were found...");
    ASSERT_ZERO(num_apic_, "I/O APIC devices should be initialized only once");

    TODO_WHEN_VMEM_WORKS
    num_apic_ = num_apic;
}

void Interrupts::InitializeIoApic(
    const size_t idx, const u8 id, const u32 address, const u32 gsi_base
)
{
    byte* ptr = mem_ + idx * sizeof(IoApic);
    new (reinterpret_cast<IoApic*>(ptr)) IoApic(id, address, gsi_base);
}
