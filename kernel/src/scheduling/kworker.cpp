#include "kworker.hpp"

#include <fs/file_descriptor.hpp>
#include <modules/scheduling.hpp>

#include "hal/debug.hpp"
#include "modules/hardware.hpp"
#include "modules/scheduling.hpp"
#include "trace_framework.hpp"

void Sched::KWorkerMain()
{
    TRACE_INFO_SCHEDULING("Created new KWorker!");

    const char *message = "KWorker is running...\n";
    // size_t message_length = strlen(message);
    // size_t written = __platform_write(kFdStdOut, message, message_length);
    // if (written != message_length) {
    //     TRACE_FATAL_SCHEDULING("KWorker failed to write to stdout!");
    // }

    char buffer[64]{};
    fd_t fd = __platform_open("/docs/greet.txt", kFdFlagReadWrite);
    if (fd < 0) {
        TRACE_FATAL_SCHEDULING("KWorker failed to open /docs/greet.txt for reading!");
    }

    int len = snprintf(
        buffer, sizeof(buffer), "Hello from KWorker nr: %llu",
        (*::SchedulingModule::Get().GetProcesses().GetCurrentProcess())->pid
    );
    ssize_t written = __platform_write(fd, buffer, len);
    if (written == -1) {
        TRACE_FATAL_SCHEDULING("KWorker failed to write to /docs/greet.txt!");
    }

    ssize_t pos = __platform_seek(fd, 0, kFdSeekSet);
    if (pos == -1) {
        TRACE_FATAL_SCHEDULING("KWorker failed to seek to start of /docs/greet.txt!");
    }

    ssize_t read = __platform_read(fd, buffer, 64);
    if (read == -1) {
        TRACE_FATAL_SCHEDULING("KWorker failed to read from /docs/greet.txt!");
    }

    TRACE_INFO_SCHEDULING("KWorker: '%.*s'", static_cast<int>(written), buffer);

    while (true) {
        static size_t kSpins = 1'000'000;
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
