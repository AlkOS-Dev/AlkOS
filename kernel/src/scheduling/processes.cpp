#include "processes.hpp"
#include "error.hpp"

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

    return process;
}

void Sched::Processes::CleanupProcess(Process *process)
{
    if (process != nullptr && process->fd_table != nullptr) {
        auto fd_table = static_cast<Fs::FdTable *>(process->fd_table);

        for (size_t i = 0; i < Fs::FdTable::kMaxFds; ++i) {
            if (fd_table->GetEntries()[i].global_entry != nullptr &&
                !fd_table->GetEntries()[i].is_standard_stream) {
                auto entry = fd_table->GetEntries()[i].global_entry;
                if (entry->file != nullptr) {
                    entry->file->ref_count--;
                    if (entry->file->ref_count == 0 && entry->file->stream != nullptr) {
                        delete entry->file->stream;
                        entry->file->stream = nullptr;
                    }
                }
                fd_table->GetEntries()[i].global_entry = nullptr;
            }
        }

        delete fd_table;
        process->fd_table = nullptr;
    }
}
