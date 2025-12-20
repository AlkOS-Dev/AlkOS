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

static DumpedRegisters g_DumpedRegisters{};

static void DivZeroExceptionHandler(intr::LitExcEntry &entry, hal::ExceptionData *data)
{
    g_DumpedRegisters.rax = data->registers.rax;
    g_DumpedRegisters.rbx = data->registers.rbx;
    g_DumpedRegisters.rcx = data->registers.rcx;
    g_DumpedRegisters.rdx = data->registers.rdx;

    g_DumpedRegisters.rsi = data->registers.rsi;
    g_DumpedRegisters.rdi = data->registers.rdi;
    g_DumpedRegisters.rbp = data->registers.rbp;
    g_DumpedRegisters.rsp = data->isr_stack_frame.rsp;

    g_DumpedRegisters.r8  = data->registers.r8;
    g_DumpedRegisters.r9  = data->registers.r9;
    g_DumpedRegisters.r10 = data->registers.r10;
    g_DumpedRegisters.r11 = data->registers.r11;
    g_DumpedRegisters.r12 = data->registers.r12;
    g_DumpedRegisters.r13 = data->registers.r13;
    g_DumpedRegisters.r14 = data->registers.r14;
    g_DumpedRegisters.r15 = data->registers.r15;

    TraceDumpedRegisters(&g_DumpedRegisters);
}

void ExceptionTestSavesAllRegisters()
{
    DumpedRegisters registers{};

    /* Ensure all register state is preserved */
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

    /* Ensure exceptions has proper data */
    HardwareModule::Get()
        .GetInterrupts()
        .GetLit()
        .InstallInterruptHandler<intr::InterruptType::kException>(
            0, {.handler = DivZeroExceptionHandler}
        );

    __asm__ volatile("movq $-1, %%rax" : : : "rax");
    __asm__ volatile("movq $-1, %%rbx" : : : "rbx");
    __asm__ volatile("movq $-1, %%rcx" : : : "rcx");
    __asm__ volatile("movq $-1, %%rdx" : : : "rdx");
    __asm__ volatile("movq $-1, %%r8" : : : "r8");
    __asm__ volatile("movq $-1, %%r9" : : : "r9");
    __asm__ volatile("movq $-1, %%r10" : : : "r10");
    __asm__ volatile("movq $-1, %%r11" : : : "r11");
    __asm__ volatile("movq $-1, %%r12" : : : "r12");
    __asm__ volatile("movq $-1, %%r13" : : : "r13");
    __asm__ volatile("movq $-1, %%r14" : : : "r14");
    __asm__ volatile("movq $-1, %%r15" : : : "r15");
    DumpedRegisters exc_registers{};
    DumpGeneralRegisters(&exc_registers);
    InvokeInterrupt<0>();
    TraceDumpedRegisters(&exc_registers);

    for (size_t idx = 0; idx <= RegisterIdx::kRegR15; ++idx) {
        R_ASSERT_EQ(
            exc_registers.flat[idx], g_DumpedRegisters.flat[idx], "Failed on idx: %zu", idx
        );
    }

    EnableHardwareInterrupts();
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
