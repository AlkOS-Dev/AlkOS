#include "kworker.hpp"

#include <fs/file_descriptor.hpp>
#include <modules/memory.hpp>
#include <modules/scheduling.hpp>
#include <modules/vfs.hpp>
#include <sys/loader.hpp>

#include "hal/debug.hpp"
#include "modules/hardware.hpp"
#include "modules/scheduling.hpp"
#include "scheduling/processes.hpp"
#include "trace_framework.hpp"

void Sched::KWorkerMain()
{
    TRACE_INFO_SCHEDULING("Created new KWorker!");

    while (true) {
        static size_t kSpins = 1'000;
        for (size_t i = 0; i < kSpins; i++) {
            hal::Noop();
            hal::Noop();
            hal::Noop();
            hal::Noop();
        }
    }
}

void Sched::TraceDumperMain()
{
    TRACE_INFO_SCHEDULING("Created new TraceDumper!");
    while (true) {
        trace::TraceDumperTask();

        HardwareModule::Get().GetInterrupts().BlockHardwareInterrupts();
        SchedulingModule::Get().GetScheduler().Yield();
        HardwareModule::Get().GetInterrupts().EnableHardwareInterrupts();
    }
}
