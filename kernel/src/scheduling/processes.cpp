// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#include "processes.hpp"
#include "error.hpp"

#include <data_structures/tagged_pointer.hpp>
#include <modules/memory.hpp>
#include <template/scope_guard.hpp>
#include "fs/file_descriptor.hpp"
#include "modules/vfs.hpp"
#include "modules/video.hpp"

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

    // Allocate wait queue
    const auto wait_queue = Mem::KNew<WaitQueue<Thread, kWaitQueueIntrusiveLevel>>();
    if (!wait_queue) {
        return std::unexpected(Error::OutOfMemory);
    }
    template_lib::BatchedScopeGuard wait_queue_guard(dismiss, [&]() {
        Mem::KDelete(wait_queue.value());
    });
    process->wait_queue = wait_queue.value();

    // Create the process's file descriptor table
    const auto fd_table_ptr = Mem::KNew<Fs::FdTable>();
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

    auto stdin_fd_result = fd_table->AllocateAt(std::move(*stdin_entry_result), Fs::kStdinFd);
    RET_UNEXPECTED_IF(!stdin_fd_result, Error::OutOfMemory);

    // Open stdout pipe in the global open file table
    auto stdout_entry_result = open_file_table.OpenPipe(process->stdout_pipe);
    RET_UNEXPECTED_IF(!stdout_entry_result, Error::OutOfMemory);

    auto stdout_fd_result = fd_table->AllocateAt(std::move(*stdout_entry_result), Fs::kStdoutFd);
    RET_UNEXPECTED_IF(!stdout_fd_result, Error::OutOfMemory);

    // Open stderr pipe in the global open file table
    auto stderr_entry_result = open_file_table.OpenPipe(process->stderr_pipe);
    RET_UNEXPECTED_IF(!stderr_entry_result, Error::OutOfMemory);

    auto stderr_fd_result = fd_table->AllocateAt(std::move(*stderr_entry_result), Fs::kStderrFd);
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

    ASSERT_NOT_NULL(process->wait_queue);
    ASSERT_TRUE(process->wait_queue->IsEmpty());
    Mem::KDelete(process->wait_queue);

    const auto result = MemoryModule::Get().GetVmm().DestroyUserAddrSpace(process->address_space);
    ASSERT_TRUE(static_cast<bool>(result));

    ::VideoModule::Get().GetWindowManager().ReleaseSession(process->pid);
}
