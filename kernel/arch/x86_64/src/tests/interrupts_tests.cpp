/* internal includes */
#include "interrupts/interrupt_types.hpp"
#include "modules/hardware.hpp"

// ------------------------------
// Preserve cpu state test
// ------------------------------

static void PolluteAllRegistersSw(intr::LitSwEntry &)
{
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

void ExceptionTestSavesAllRegisters()
{
    DumpedRegisters registers{};

    const u16 sw_lirq = HardwareModule::Get().GetInterrupts().GetLit().InstallSwIntrFirstFree(
        {.handler = PolluteAllRegistersSw}
    );
    R_ASSERT_NEQ(sw_lirq, std::numeric_limits<u16>::max());
    const u8 hw_intr = HardwareModule::Get()
                           .GetInterrupts()
                           .GetLit()
                           .GetEntry<intr::InterruptType::kSoftwareInterrupt>(sw_lirq)
                           .hardware_irq;

    BlockHardwareInterrupts();
    InvokeInterruptDynamic(hw_intr);
    DumpRegisters(&registers);

    for (size_t idx = 0; idx <= RegisterIdx::kRegR15; ++idx) {
        const u64 content = registers.flat[idx];
        R_ASSERT_NEQ(kBitMask64, content);
    }
}

/**
 *  @brief Test should simply drop 0 division exception
 */
void ExceptionFailsKernelTest()
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiv-by-zero"
    [[maybe_unused]] volatile int a = 9 / 0;
#pragma GCC diagnostic pop
}
