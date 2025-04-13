#include "interrupts.hpp"
#include "drivers/pic8259/pic8259.hpp"
#include "interrupts/idt.hpp"

#include <extensions/debug.hpp>

using namespace arch;

void Interrupts::Initialise() { TRACE_INFO("Initialising interrupts system..."); }

void Interrupts::FirstStageInit()
{
    TRACE_INFO("Interrupts first stage init...");

    InitPic8259(kIrq1Offset, kIrq2Offset);
    InitializeDefaultIdt_();
}
