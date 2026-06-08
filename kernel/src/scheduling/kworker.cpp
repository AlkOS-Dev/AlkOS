#include "kworker.hpp"

#include <fs/file_descriptor.hpp>
#include <modules/memory.hpp>
#include <modules/scheduling.hpp>
#include <modules/vfs.hpp>
#include <sys/loader.hpp>
#include <syscalls/calls/thread.hpp>

#include "hal/debug.hpp"
#include "modules/hardware.hpp"
#include "modules/scheduling.hpp"
#include "scheduling/processes.hpp"
#include "trace_framework.hpp"

void Sched::TraceDumperMain()
{
    TRACE_INFO_SCHEDULING("Created new TraceDumper!");

    while (true) {
        trace::TraceDumperTask();
        SchedulingModule::Get().GetScheduler().Yield();
    }
}

void Sched::ThreadRipperMain()
{
    TRACE_INFO_SCHEDULING("Created new ThreadRipper!");

    while (true) {
        SchedulingModule::Get().GetTaskMgr().ThreadRipperWork();
        SchedulingModule::Get().GetScheduler().Yield();
    }
}

void Sched::ProcessRipperMain()
{
    TRACE_INFO_SCHEDULING("Created new ProcessRipper!");

    while (true) {
        SchedulingModule::Get().GetTaskMgr().ProcessRipperWork();
        SchedulingModule::Get().GetScheduler().Yield();
    }
}

void Sched::StdoutTracerMain(Pid pid)
{
    TRACE_INFO_SCHEDULING("Created new StdoutTracer!");

    // Get hello process
    auto &processes = SchedulingModule::Get().GetProcesses();
    auto res        = processes.GetProcess(pid);
    R_ASSERT_TRUE(static_cast<bool>(res), "Failed to find process for tracing...");
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

        SchedulingModule::Get().GetScheduler().Yield();
    }
}
