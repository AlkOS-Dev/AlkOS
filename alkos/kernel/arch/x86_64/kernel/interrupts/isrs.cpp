#include "drivers/apic/local_apic.hpp"
#include "drivers/pic8259/pic8259.hpp"
#include "interrupts/idt.hpp"
#include "modules/hardware.hpp"

#include <extensions/debug.hpp>

/**
 * @brief Logs the received interrupt.
 *
 * @param stack_frame Pointer to the ISR stack frame.
 * @param idt_idx index of interrupt triggered.
 */
void LogIrqReceived([[maybe_unused]] void *stack_frame, const u8 idt_idx)
{
    TRACE_INFO("Received interrupt with idx: %hhu\n", idt_idx);
}

extern "C" {
// ------------------------------
// IRQ0: System timer
// ------------------------------

void isr_32([[maybe_unused]] void *const stack_frame)
{
    LogIrqReceived(stack_frame, 32);
    HardwareModule::Get().GetInterrupts().GetLocalApic().IsEnabled() ? LocalApic::SendEOI()
                                                                     : Pic8259SendEOI(0);

    if (HardwareModule::Get().GetInterrupts().GetHpet()) {
        TRACE_DEBUG(
            "HPET COUNTER: %zu", HardwareModule::Get().GetInterrupts().GetHpet()->ReadMainCounter()
        );
    }
}

// ------------------------------
// IRQ1: keyboard
// ------------------------------

void isr_33(void *const stack_frame)
{
    LogIrqReceived(stack_frame, 33);
    HardwareModule::Get().GetInterrupts().GetLocalApic().IsEnabled() ? LocalApic::SendEOI()
                                                                     : Pic8259SendEOI(1);
}

// ------------------------------
// IRQ3: COM2
// ------------------------------

void isr_35(void *const stack_frame)
{
    LogIrqReceived(stack_frame, 35);
    HardwareModule::Get().GetInterrupts().GetLocalApic().IsEnabled() ? LocalApic::SendEOI()
                                                                     : Pic8259SendEOI(3);
}

// ------------------------------
// IRQ4: COM1
// ------------------------------

void isr_36(void *const stack_frame)
{
    LogIrqReceived(stack_frame, 36);
    HardwareModule::Get().GetInterrupts().GetLocalApic().IsEnabled() ? LocalApic::SendEOI()
                                                                     : Pic8259SendEOI(4);
}

// ------------------------------
// IRQ8: real time clock
// ------------------------------

void isr_40(void *const stack_frame)
{
    LogIrqReceived(stack_frame, 40);
    HardwareModule::Get().GetInterrupts().GetLocalApic().IsEnabled() ? LocalApic::SendEOI()
                                                                     : Pic8259SendEOI(8);
}

// ------------------------------
// IRQ12: mouse
// ------------------------------

void isr_44(void *const stack_frame)
{
    LogIrqReceived(stack_frame, 44);
    HardwareModule::Get().GetInterrupts().GetLocalApic().IsEnabled() ? LocalApic::SendEOI()
                                                                     : Pic8259SendEOI(12);
}

// ------------------------------
// Test isr
// ------------------------------

void isr_48(void *const stack_frame)
{
    LogIrqReceived(stack_frame, 48);

    /* pollute all registers possible */
    __asm__ volatile("movq $-1, %%rax" : : : "rax");
    __asm__ volatile("movq $-1, %%rbx" : : : "rbx");
    __asm__ volatile("movq $-1, %%rcx" : : : "rcx");
    __asm__ volatile("movq $-1, %%rdx" : : : "rdx");
    __asm__ volatile("movq $-1, %%rsi" : : : "rsi");
    __asm__ volatile("movq $-1, %%rdi" : : : "rdi");
    __asm__ volatile("movq $-1, %%r8" : : : "r8");
    __asm__ volatile("movq $-1, %%r9" : : : "r9");
    __asm__ volatile("movq $-1, %%r10" : : : "r10");
    __asm__ volatile("movq $-1, %%r11" : : : "r11");
    __asm__ volatile("movq $-1, %%r12" : : : "r12");
    __asm__ volatile("movq $-1, %%r13" : : : "r13");
    __asm__ volatile("movq $-1, %%r14" : : : "r14");
    __asm__ volatile("movq $-1, %%r15" : : : "r15");
}
}
