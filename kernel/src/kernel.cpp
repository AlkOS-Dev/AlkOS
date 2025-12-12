#include <assert.h>
#include <autogen/feature_flags.h>
#include <time.h>
#include <string.hpp>
#include <test_module/test_module.hpp>
#include "trace_framework.hpp"

/* internal includes */
#include <hal/debug.hpp>

#include "graphics/font/psf2_font.hpp"
#include "graphics/fonts/drdos8x8.hpp"
#include "graphics/painter.hpp"
#include "hal/terminal.hpp"
#include "modules/hardware.hpp"
#include "modules/video.hpp"
#include "sys/shell.hpp"

#include "boot_args.hpp"
#include "drivers/apic/local_apic.hpp"
#include "hal/boot_args.hpp"
#include "io/register.hpp"  // For port I/O
#include "mem/heap.hpp"
#include "modules/hardware.hpp"
#include "todo.hpp"

void DebugKeyboardState()
{
    using namespace hal;

    // 1. Check PS/2 Status Register (Port 0x64)
    // Bit 0 = Output Buffer Full. If 1, data is waiting.
    u8 status = arch::IoRead<u8>(0x64);
    DEBUG_INFO_GENERAL("PS/2 Status: 0x%02x (OBF: %d)", status, status & 1);

    // 2. Check Local APIC ID and ISR
    // We want to see if the CPU thinks it's servicing an interrupt
    auto &lapic = HardwareModule::Get().GetInterrupts().GetLocalApic();
    u32 id      = lapic.ReadRegister(LocalApic::kIdRegRW);
    u32 isr     = lapic.ReadRegister(0x110);  // ISR for vectors 32-63
    u32 irr     = lapic.ReadRegister(0x210);  // IRR for vectors 32-63
    DEBUG_INFO_GENERAL("LAPIC ID: %u, ISR[32-63]: 0x%08x, IRR[32-63]: 0x%08x", id >> 24, isr, irr);

    // 3. Check IOAPIC Redirection Entry for IRQ 1
    // Usually IOAPIC ID 0 is the one handling ISA IRQs.
    // GSI 1 corresponds to the keyboard.
    // We need to access the registers manually here for debug if no getter exists.
    auto &ioapic_table = HardwareModule::Get().GetInterrupts().GetIoApicTable();
    if (ioapic_table.Size() > 0) {
        auto &ioapic = ioapic_table[0];  // Assuming first IOAPIC covers GSI 0-23

        // Read entry 1 (keyboard)
        // Redirection entries start at 0x10, 2 registers per entry (low/high)
        auto low  = ioapic.ReadRegister(0x10 + (1 * 2));
        auto high = ioapic.ReadRegister(0x10 + (1 * 2) + 1);

        DEBUG_INFO_GENERAL("IOAPIC Redir[1] Low: 0x%08x, High: 0x%08x", low, high);
        DEBUG_INFO_GENERAL("  -> Vector: 0x%02x", low & 0xFF);
        DEBUG_INFO_GENERAL("  -> Masked: %d", (low >> 16) & 1);
        DEBUG_INFO_GENERAL("  -> Trigger: %s", ((low >> 15) & 1) ? "Level" : "Edge");
        DEBUG_INFO_GENERAL("  -> Dest: %u", (high >> 24));
    }
}

extern void KernelInit(const hal::RawBootArguments &);

static void KernelRun()
{
    TRACE_INFO_GENERAL("Hello from AlkOS!");
    trace::Flush();

    auto &video = VideoModule::Get();
    Graphics::Painter painter(video.GetScreen(), video.GetFormat());
    Graphics::Psf2Font font(drdos8x8_psfu);

    if (!font.IsValid()) {
        TRACE_WARN_VIDEO("Invalid font");
    }

    System::GraphicsConsole console(painter, font);
    System::Shell shell(console, HardwareModule::Get().GetPs2Keyboard());

    shell.Init();
    video.Flush();

    while (true) {
        shell.Update();
        video.Flush();

        for (volatile i32 i = 0; i < 10000; ++i) {
        }
        trace::Flush();
    }
}

extern "C" void KernelMain(const hal::RawBootArguments *raw_args)
{
    ASSERT_NOT_NULL(raw_args, "Raw boot arguments are null");
    TRACE_INFO_GENERAL("Running kernel initialization...");

    hal::DebugStack();
    KernelInit(*raw_args);

    if constexpr (FeatureEnabled<FeatureFlag::kRunTestMode>) {
        TRACE_INFO_GENERAL("Running tests...");
        test::TestModule test_module{};
        test_module.RunTestModule();
        R_ASSERT(false && "Test module should never exit!");
    }

    TRACE_INFO_GENERAL("Proceeding to DebugKeyboardState...");
    DebugKeyboardState();

    TRACE_INFO_GENERAL("Proceeding to KernelRun...");
    KernelRun();
}
