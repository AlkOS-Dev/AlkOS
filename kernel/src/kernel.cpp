#include <assert.h>
#include <autogen/feature_flags.h>
#include <time.h>
#include <string.hpp>
#include <test_module/test_module.hpp>
#include "trace_framework.hpp"

/* internal includes */
#include <hal/debug.hpp>

#include "graphics/donut/donut.hpp"
#include "graphics/font/psf2_font.hpp"
#include "graphics/fonts/drdos8x8.hpp"
#include "graphics/painter.hpp"

#include "boot_args.hpp"
#include "hal/boot_args.hpp"
#include "mem/heap.hpp"
#include "modules/video.hpp"
#include "todo.hpp"

extern void KernelInit(const hal::RawBootArguments &);

static void KernelRun()
{
    TRACE_INFO_GENERAL("Hello from AlkOS!");
    trace::DumpAllBuffersOnFailure();
    TODO_MMU_MINIMAL

    // static constexpr size_t kBuffSize = 256;
    // char buff[kBuffSize];
    //
    // const auto t = time(nullptr);
    // strftime(buff, kBuffSize, "%Y-%m-%d %H:%M:%S", localtime(&t));
    //
    // KernelTraceSuccess("Hello from AlkOS! Today we have: %s", buff);

    auto &video  = VideoModule::Get();
    auto &screen = video.GetScreen();
    auto fmt     = video.GetFormat();

    Graphics::Painter p(screen, fmt);
    Graphics::Psf2Font system_font(drdos8x8_psfu);

    if (!system_font.IsValid()) {
        TRACE_WARN_VIDEO("System font magic invalid! Rendering might be corrupted.");
    }

    SpinningDonut donut;
    donut.Init(screen.GetWidth(), screen.GetHeight());

    u64 frame_count = 0;
    char text_buffer[100];
    while (true) {
        donut.Render(screen, p);

        p.SetColor(Graphics::Color::White());
        p.DrawString(
            {.x = 10, .y = 10, .text = "AlkOS Kernel - Graphics Module ON", .scale = 2}, system_font
        );
        int ret = snprintf(text_buffer, 100, "Frame %lli", frame_count);
        p.DrawString(
            {.x     = (static_cast<i32>(
                 screen.GetWidth() - system_font.MeasureString(text_buffer).width * 2
             )),
             .y     = 10,
             .text  = text_buffer,
             .scale = 2},
            system_font
        );
        video.Flush();

        for (volatile int i = 0; i < 10000000; i++) {
            ;
        }

        frame_count++;
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
