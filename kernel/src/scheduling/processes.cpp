#include "processes.hpp"
#include "error.hpp"

#include <data_structures/tagged_pointer.hpp>
#include <template/scope_guard.hpp>
#include "fs/file_descriptor.hpp"

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

    // TODO: Insert the references when cloning processes (fork, etc.) or store them somewhere else
    auto stdin_entry = fd_table_ptr->GetEntry(Fs::kStdinFd);
    *stdin_entry     = Fs::FdTable::FdEntry::Construct<IO::Pipe<4096>>();
    RET_UNEXPECTED_IF(!stdin_entry, Error::OutOfMemory);

    auto stdout_entry = fd_table_ptr->GetEntry(Fs::kStdoutFd);
    *stdout_entry     = Fs::FdTable::FdEntry::Construct<IO::Pipe<4096>>();
    RET_UNEXPECTED_IF(!stdout_entry, Error::OutOfMemory);

    auto stderr_entry = fd_table_ptr->GetEntry(Fs::kStderrFd);
    *stderr_entry     = Fs::FdTable::FdEntry::Construct<IO::Pipe<4096>>();
    RET_UNEXPECTED_IF(!stderr_entry, Error::OutOfMemory);

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
