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

void Sched::StdoutTracerMain(Pid pid)
{
    TRACE_INFO_SCHEDULING("Created new StdoutTracer!");

    // Get hello process
    auto &processes = SchedulingModule::Get().GetProcesses();
    auto res        = processes.GetProcess(pid);
    R_ASSERT_TRUE(static_cast<bool>(res), "Failed to find hello_world process for tracing...");
    auto *hello_process = res.value();

    // Read from the stdout pipe and write to terminal
    byte buffer[256];
    while (true) {
        auto result = hello_process->stdout_pipe.Read(std::span<byte>(buffer, sizeof(buffer)));

        if (result.has_value()) {
            size_t bytes_read = result.value();
            if (bytes_read > 0) {
                size_t safe_size =
                    bytes_read < sizeof(buffer) - 1 ? bytes_read : sizeof(buffer) - 1;
                buffer[safe_size] = '\0';
                hal::TerminalWriteString(reinterpret_cast<const char *>(buffer));
            }
        }

        HardwareModule::Get().GetInterrupts().BlockHardwareInterrupts();
        SchedulingModule::Get().GetScheduler().Yield();
        HardwareModule::Get().GetInterrupts().EnableHardwareInterrupts();
    }
}
