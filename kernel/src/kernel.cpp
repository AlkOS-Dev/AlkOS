#include <assert.h>
#include <autogen/feature_flags.h>
#include <string.hpp>
#include <test_module/test_module.hpp>
#include "trace_framework.hpp"

/* internal includes */
#include <hal/debug.hpp>
#include <hal/terminal.hpp>

#include "graphics/font/psf2_font.hpp"
#include "graphics/fonts/drdos8x8.hpp"
#include "graphics/painter.hpp"
#include "modules/hardware.hpp"
#include "modules/video.hpp"
#include "sys/shell.hpp"

#include "drivers/apic/local_apic.hpp"
#include "hal/boot_args.hpp"
#include "modules/hardware.hpp"
#include "modules/scheduling.hpp"
#include "todo.hpp"

extern void KernelInit(const hal::RawBootArguments &);

static void KernelRun()
{
    TRACE_INFO_GENERAL("Hello from AlkOS!");
    trace::DumpAllBuffersOnFailure();  // TODO: should be removed and testing converted to threads?

    SchedulingModule::Get().GetScheduler().ConvertToScheduling();
}

extern "C" void KernelMain(const Mem::PPtr<hal::RawBootArguments> raw_args)
{
    ASSERT_NOT_NULL(raw_args, "Raw boot arguments are null");
    TRACE_INFO_GENERAL("Running kernel initialization...");

    KernelInit(*Mem::PhysToVirt(raw_args));

    if constexpr (FeatureEnabled<FeatureFlag::kRunTestMode>) {
        TRACE_INFO_GENERAL("Running tests...");
        test::TestModule test_module{};
        test_module.RunTestModule();
        R_ASSERT(false && "Test module should never exit!");
    }

    TRACE_INFO_GENERAL("Proceeding to KernelRun...");
    KernelRun();
}
