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
#include "modules/video.hpp"
#include "sys/shell.hpp"

#include "boot_args.hpp"
#include "hal/boot_args.hpp"
#include "mem/heap.hpp"
#include "todo.hpp"

extern void KernelInit(const hal::RawBootArguments &);

static void KernelRun()
{
    TRACE_INFO_GENERAL("Hello from AlkOS!");
    trace::DumpAllBuffersOnFailure();
    TODO_MMU_MINIMAL

    auto &video = VideoModule::Get();
    Graphics::Painter painter(video.GetScreen(), video.GetFormat());
    Graphics::Psf2Font font(drdos8x8_psfu);

    if (!font.IsValid()) {
        TRACE_WARN_VIDEO("Invalid font");
    }

    System::GraphicsConsole console(painter, font);
    System::Shell shell(console);

    shell.Init();
    video.Flush();

    while (true) {
        // Poll Serial Port for input (temporary until IRQ is hooked)
        char c = hal::TerminalGetChar();
        shell.OnInput(c);
        video.Flush();

        // Don't burn CPU 100%
        for (volatile i32 i = 0; i < 10000; ++i) {
        }
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

    TRACE_INFO_GENERAL("Proceeding to KernelRun...");
    KernelRun();
}
