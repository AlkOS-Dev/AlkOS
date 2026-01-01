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

void Sched::FdHierarchyDumperMain()
{
    TRACE_INFO_SCHEDULING("Created new FdHierarchyDumper!");

    while (true) {
        DEBUG_INFO_VFS("===== Three-Way FD Hierarchy Debug Info =====");

        auto &fd_manager      = VfsModule::Get().GetFdManager();
        auto &file_table      = fd_manager.GetFileTable();
        auto &open_file_table = fd_manager.GetOpenFileTable();

        DEBUG_INFO_VFS("");
        DEBUG_INFO_VFS("Level 1: Global File Table (Files system-wide)");
        DEBUG_INFO_VFS("------------------------------------------------");

        size_t file_count = 0;
        for (size_t i = 0; i < Fs::kMaxActiveFiles; ++i) {
            const Fs::File *file = file_table.GetFile(i);
            if (file != nullptr && file->HasRefs()) {
                DEBUG_INFO_VFS(
                    "  File[%zu]: path='%s', size=%llu, mode=0x%08X, ref_count=%u", i,
                    file->path.CString(), file->size, file->mode, file->GetRefCount()
                );
                file_count++;
            }
        }
        DEBUG_INFO_VFS("  Total active files: %zu", file_count);

        DEBUG_INFO_VFS("");
        DEBUG_INFO_VFS("Level 2: Global Open File Table (Open entries system-wide)");
        DEBUG_INFO_VFS("----------------------------------------------------------");

        size_t open_file_count = 0;
        for (size_t i = 0; i < Fs::kMaxOpenFiles; ++i) {
            const Fs::OpenFileEntry *entry = open_file_table.GetEntry(i);
            if (entry != nullptr && entry->HasRefs()) {
                const char *type_str = entry->IsFile() ? "File" : "Pipe";
                DEBUG_INFO_VFS(
                    "  OpenFile[%zu]: type=%s, flags=0x%08X, offset=%llu, ref_count=%u, "
                    "is_append=%s",
                    i, type_str, entry->flags, entry->offset, entry->GetRefCount(),
                    entry->is_append ? "true" : "false"
                );
                open_file_count++;
            }
        }
        DEBUG_INFO_VFS("  Total open file entries: %zu", open_file_count);

        DEBUG_INFO_VFS("");
        DEBUG_INFO_VFS("Level 3: Process FD Tables (Per-process file descriptors)");
        DEBUG_INFO_VFS("-----------------------------------------------------------");

        auto &processes      = SchedulingModule::Get().GetProcesses();
        size_t total_fds     = 0;
        size_t process_count = 0;

        for (size_t pid = 0; pid < kMaxProcesses; ++pid) {
            Pid current_pid;
            current_pid.id    = static_cast<u16>(pid);
            current_pid.count = 0;

            auto process = processes.GetProcess(current_pid);
            if (process && process.value()->fd_table != nullptr) {
                Fs::FdTable *fd_table = process.value()->fd_table;
                DEBUG_INFO_VFS(
                    "  Process %s (pid=%zu): open_count=%zu", process.value()->name, pid,
                    fd_table->GetOpenCount()
                );

                size_t process_fds = 0;
                for (size_t fd = 0; fd < Fs::kMaxFdsPerProcess; ++fd) {
                    Fs::OpenFileEntry *entry = fd_table->GetEntry(fd);
                    if (entry != nullptr) {
                        const char *type_str = entry->IsFile() ? "File" : "Pipe";
                        DEBUG_INFO_VFS(
                            "    fd=%zu -> type=%s, open_entry@%p, ref_count=%u", fd, type_str,
                            entry, entry->GetRefCount()
                        );
                        process_fds++;
                    }
                }
                total_fds += process_fds;
                process_count++;
            }
        }
        DEBUG_INFO_VFS("  Total processes with FD tables: %zu", process_count);
        DEBUG_INFO_VFS("  Total file descriptors across all processes: %zu", total_fds);

        DEBUG_INFO_VFS("");
        DEBUG_INFO_VFS("===== End of FD Hierarchy Debug Info =====");
        DEBUG_INFO_VFS("");

        HardwareModule::Get().GetInterrupts().BlockHardwareInterrupts();
        SchedulingModule::Get().GetScheduler().Yield();
        HardwareModule::Get().GetInterrupts().EnableHardwareInterrupts();
    }
}
