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
    const size_t idx = processes_.Allocate();

    if (idx == std::numeric_limits<size_t>::max()) {
        return std::unexpected(Error::ExceededMaxAllowedInstances);
    }

    Process *process = processes_.Get(idx);

    ASSERT_LE(idx, std::numeric_limits<u16>::max());
    process->pid = AssignNewPid(static_cast<u16>(idx));

    auto fd_table = Mem::KMalloc<Fs::FdTable>();
    RET_UNEXPECTED_IF(!fd_table, Error::OutOfMemory);

    auto fd_table_ptr = new (*fd_table) Fs::FdTable();
    process->fd_table = fd_table_ptr;

    template_lib::ScopeGuard fd_table_guard([&]() {
        fd_table_ptr->~FdTable();
        Mem::KFree(fd_table_ptr);
    });

    // Get the global OpenFileTable
    auto &open_file_table = VfsModule::Get().GetFdManager().GetOpenFileTable();

    // Open stdin pipe in the global open file table
    auto stdin_entry_result = open_file_table.OpenPipe(process->stdin_pipe);
    RET_UNEXPECTED_IF(!stdin_entry_result, Error::OutOfMemory);
    auto *stdin_entry = *stdin_entry_result;

    // Allocate stdin file descriptor
    auto stdin_fd_result = fd_table_ptr->AllocateFdAt(stdin_entry, Fs::kStdinFd);
    if (!stdin_fd_result) {
        open_file_table.CloseFile(stdin_entry);
        return std::unexpected(Error::OutOfMemory);
    }

    // Open stdout pipe in the global open file table
    auto stdout_entry_result = open_file_table.OpenPipe(process->stdout_pipe);
    if (!stdout_entry_result) {
        open_file_table.CloseFile(stdin_entry);
        return std::unexpected(Error::OutOfMemory);
    }
    auto *stdout_entry = *stdout_entry_result;

    // Allocate stdout file descriptor
    auto stdout_fd_result = fd_table_ptr->AllocateFdAt(stdout_entry, Fs::kStdoutFd);
    if (!stdout_fd_result) {
        open_file_table.CloseFile(stdin_entry);
        open_file_table.CloseFile(stdout_entry);
        return std::unexpected(Error::OutOfMemory);
    }

    // Open stderr pipe in the global open file table
    auto stderr_entry_result = open_file_table.OpenPipe(process->stderr_pipe);
    if (!stderr_entry_result) {
        open_file_table.CloseFile(stdin_entry);
        open_file_table.CloseFile(stdout_entry);
        return std::unexpected(Error::OutOfMemory);
    }
    auto *stderr_entry = *stderr_entry_result;

    // Allocate stderr file descriptor
    auto stderr_fd_result = fd_table_ptr->AllocateFdAt(stderr_entry, Fs::kStderrFd);
    if (!stderr_fd_result) {
        open_file_table.CloseFile(stdin_entry);
        open_file_table.CloseFile(stdout_entry);
        open_file_table.CloseFile(stderr_entry);
        return std::unexpected(Error::OutOfMemory);
    }

    fd_table_guard.dismiss();

    return process;
}

void Sched::Processes::CleanupProcess(Process *process)
{
    ASSERT_NOT_NULL(process);

    auto fd_table = process->fd_table;
    ASSERT_NOT_NULL(fd_table);

    fd_table->~FdTable();
    Mem::KFree(fd_table);
}
