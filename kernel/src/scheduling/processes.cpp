#include "processes.hpp"
#include "error.hpp"

#include <data_structures/tagged_pointer.hpp>
#include <template/scope_guard.hpp>
#include "fs/file_descriptor.hpp"
#include "modules/vfs.hpp"

// ------------------------------
// statics
// ------------------------------

// ------------------------------
// Implementations
// ------------------------------

std::expected<Sched::Process *, Sched::Error> Sched::Processes::PrepareProcess()
{
    bool dismiss = false;

    const size_t idx = processes_.Allocate();

    template_lib::BatchedScopeGuard process_guard(dismiss, [&]() {
        processes_.Free(idx);
    });

    if (idx == std::numeric_limits<size_t>::max()) {
        return std::unexpected(Error::ExceededMaxAllowedInstances);
    }

    Process *process = processes_.Get(idx);

    ASSERT_LE(idx, std::numeric_limits<u16>::max());
    process->pid = AssignNewPid(static_cast<u16>(idx));

    // ----------------------------------------------------------
    // Initialize standard I/O pipes
    // ----------------------------------------------------------

    // Create the process's file descriptor table
    auto fd_table_ptr = Mem::KNew<Fs::FdTable>();
    RET_UNEXPECTED_IF(!fd_table_ptr, Error::OutOfMemory);

    auto *fd_table = process->fd_table = *fd_table_ptr;
    template_lib::BatchedScopeGuard fd_table_guard(dismiss, [&]() {
        Mem::KDelete(fd_table);
    });

    // Get the global OpenFileTable
    auto &open_file_table = VfsModule::Get().GetFdManager().GetOpenFileTable();

    // Open stdin pipe in the global open file table
    auto stdin_entry_result = open_file_table.OpenPipe(process->stdin_pipe);
    RET_UNEXPECTED_IF(!stdin_entry_result, Error::OutOfMemory);

    auto *stdin_entry = *stdin_entry_result;
    template_lib::BatchedScopeGuard stdin_guard(dismiss, [&]() {
        open_file_table.CloseEntry(stdin_entry);
    });

    auto stdin_fd_result = fd_table->AllocateFdAt(stdin_entry, Fs::kStdinFd);
    RET_UNEXPECTED_IF(!stdin_fd_result, Error::OutOfMemory);

    // Open stdout pipe in the global open file table
    auto stdout_entry_result = open_file_table.OpenPipe(process->stdout_pipe);
    RET_UNEXPECTED_IF(!stdout_entry_result, Error::OutOfMemory);

    auto *stdout_entry = *stdout_entry_result;
    template_lib::BatchedScopeGuard stdout_guard(dismiss, [&]() {
        open_file_table.CloseEntry(stdout_entry);
    });

    auto stdout_fd_result = fd_table->AllocateFdAt(stdout_entry, Fs::kStdoutFd);
    RET_UNEXPECTED_IF(!stdout_fd_result, Error::OutOfMemory);

    // Open stderr pipe in the global open file table
    auto stderr_entry_result = open_file_table.OpenPipe(process->stderr_pipe);
    RET_UNEXPECTED_IF(!stderr_entry_result, Error::OutOfMemory);

    auto *stderr_entry = *stderr_entry_result;
    template_lib::BatchedScopeGuard stderr_guard(dismiss, [&]() {
        open_file_table.CloseEntry(stderr_entry);
    });

    auto stderr_fd_result = fd_table->AllocateFdAt(stderr_entry, Fs::kStderrFd);
    RET_UNEXPECTED_IF(!stderr_fd_result, Error::OutOfMemory);

    dismiss = true;

    return process;
}

void Sched::Processes::CleanupProcess(Process *process)
{
    ASSERT_NOT_NULL(process);

    auto fd_table = process->fd_table;
    ASSERT_NOT_NULL(fd_table);

    Mem::KDelete(fd_table);
}
