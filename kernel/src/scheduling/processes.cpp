#include "processes.hpp"
#include "error.hpp"

#include <data_structures/tagged_pointer.hpp>
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
    if (fd_table == nullptr) {
        return std::unexpected(Error::OutOfMemory);
    }
    process->fd_table = new (*fd_table) Fs::FdTable();

    // Initialize standard stream file descriptors using TaggedPointer
    // The pipes are allocated directly in fd_table entries
    auto fd_table_ptr = static_cast<Fs::FdTable *>(process->fd_table);

    // Allocate fd 0 (stdin) with pipe
    auto stdin_entry = fd_table_ptr->GetEntry(Fs::kStdinFd);
    *stdin_entry = data_structures::TaggedPointer<Fs::OpenFileEntry *, IO::Pipe<4096>>::Construct<
        IO::Pipe<4096>>();
    if (!stdin_entry->IsValid()) {
        delete fd_table_ptr;
        process->fd_table = nullptr;
        return std::unexpected(Error::OutOfMemory);
    }

    // Allocate fd 1 (stdout) with pipe
    auto stdout_entry = fd_table_ptr->GetEntry(Fs::kStdoutFd);
    *stdout_entry = data_structures::TaggedPointer<Fs::OpenFileEntry *, IO::Pipe<4096>>::Construct<
        IO::Pipe<4096>>();
    if (!stdout_entry->IsValid()) {
        delete fd_table_ptr;
        process->fd_table = nullptr;
        return std::unexpected(Error::OutOfMemory);
    }

    // Allocate fd 2 (stderr) with pipe
    auto stderr_entry = fd_table_ptr->GetEntry(Fs::kStderrFd);
    *stderr_entry = data_structures::TaggedPointer<Fs::OpenFileEntry *, IO::Pipe<4096>>::Construct<
        IO::Pipe<4096>>();
    if (!stderr_entry->IsValid()) {
        delete fd_table_ptr;
        process->fd_table = nullptr;
        return std::unexpected(Error::OutOfMemory);
    }

    return process;
}

void Sched::Processes::CleanupProcess(Process *process)
{
    if (process != nullptr && process->fd_table != nullptr) {
        auto fd_table = static_cast<Fs::FdTable *>(process->fd_table);

        // Clean up fd_table (TaggedPointer handles cleanup automatically)
        delete fd_table;
        process->fd_table = nullptr;
    }
}
