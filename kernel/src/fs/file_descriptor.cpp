#include "file_descriptor.hpp"
#include "modules/scheduling.hpp"

#include <mutex.hpp>
#include <vfs.hpp>

#include "scheduling/processes.hpp"

namespace
{

bool IsValidFd(fd_t fd) { return fd >= 0 && static_cast<size_t>(fd) < Fs::kMaxFdsPerProcess; }

}  // namespace

namespace Fs
{

// ============================================================================
// FdTable Implementation
// ============================================================================

FdResult<fd_t> FdTable::AllocateFd(OpenFileEntry *global_entry)
{
    ASSERT_NOT_NULL(global_entry);

    std::lock_guard lock(lock_);

    for (size_t i = 0; i < kMaxFdsPerProcess; ++i) {
        if (entries_[i] == nullptr) {
            entries_[i] = global_entry;
            global_entry->AddRef();
            open_count_++;
            return static_cast<fd_t>(i);
        }
    }

    return std::unexpected(FdError::kFdTableFull);
}

FdResult<fd_t> FdTable::AllocateFdAt(OpenFileEntry *global_entry, fd_t fd)
{
    ASSERT_NOT_NULL(global_entry);

    RET_UNEXPECTED_IF(!IsValidFd(fd), FdError::kInvalidFd);

    std::lock_guard lock(lock_);

    RET_UNEXPECTED_IF(entries_[fd] != nullptr, FdError::kBadFileDescriptor);

    entries_[fd] = global_entry;
    global_entry->AddRef();
    open_count_++;

    return fd;
}

FdResult<> FdTable::FreeFd(fd_t fd)
{
    RET_UNEXPECTED_IF(!IsValidFd(fd), FdError::kInvalidFd);

    std::lock_guard lock(lock_);

    if (fd <= kStderrFd && entries_[fd] != nullptr && entries_[fd]->IsPipe()) {
        return std::unexpected(FdError::kPermissionDenied);
    }

    RET_UNEXPECTED_IF(entries_[fd] == nullptr, FdError::kNotOpen);

    entries_[fd] = nullptr;
    open_count_--;

    return {};
}

OpenFileEntry *FdTable::GetEntry(fd_t fd)
{
    if (!IsValidFd(fd)) {
        return nullptr;
    }
    return entries_[fd];
}

const OpenFileEntry *FdTable::GetEntry(fd_t fd) const
{
    if (!IsValidFd(fd)) {
        return nullptr;
    }
    return entries_[fd];
}

FdResult<fd_t> FdTable::DuplicateFd(fd_t fd)
{
    RET_UNEXPECTED_IF(!IsValidFd(fd), FdError::kInvalidFd);

    std::lock_guard lock(lock_);

    RET_UNEXPECTED_IF(entries_[fd] == nullptr, FdError::kBadFileDescriptor);

    for (size_t i = 0; i < kMaxFdsPerProcess; ++i) {
        if (entries_[i] == nullptr) {
            entries_[i] = entries_[fd];
            entries_[i]->AddRef();
            open_count_++;

            return static_cast<fd_t>(i);
        }
    }

    return std::unexpected(FdError::kFdTableFull);
}

FdResult<fd_t> FdTable::DuplicateFdTo(fd_t old_fd, fd_t new_fd)
{
    RET_UNEXPECTED_IF(!IsValidFd(old_fd), FdError::kInvalidFd);
    RET_UNEXPECTED_IF(!IsValidFd(new_fd), FdError::kInvalidFd);

    std::lock_guard lock(lock_);

    RET_UNEXPECTED_IF(entries_[old_fd] == nullptr, FdError::kBadFileDescriptor);

    // If new_fd is already open, close it first
    if (entries_[new_fd] != nullptr) {
        entries_[new_fd] = nullptr;
        open_count_--;
    }

    entries_[new_fd] = entries_[old_fd];
    entries_[new_fd]->AddRef();
    open_count_++;

    return new_fd;
}

// ============================================================================
// FileTable Implementation
// ============================================================================

FdResult<File *> FileTable::GetOrCreate(const vfs::Path &path)
{
    RET_UNEXPECTED_IF(path.IsEmpty(), FdError::kInvalidArgument);

    std::lock_guard lock(lock_);

    File *existing = Find(path);
    if (existing != nullptr) {
        return existing;
    }

    size_t idx = files_.Allocate();
    RET_UNEXPECTED_IF(idx == std::numeric_limits<size_t>::max(), FdError::kIoError);

    File *file = files_.Get(idx);
    ASSERT_NOT_NULL(file);

    new (file) File();

    // TODO: Get from VFS when implemented
    file->size = 0;
    file->mode = 0;
    file->path = path;

    count_++;
    return file;
}

FdResult<> FileTable::Release(File *file)
{
    if (file == nullptr) {
        return std::unexpected(FdError::kInvalidArgument);
    }

    std::lock_guard lock(lock_);

    if (!file->HasRefs()) {
        return std::unexpected(FdError::kNotOpen);
    }

    u32 old_count = file->GetRefCount();
    file->Release();

    if (old_count == 1) {
        for (size_t i = 0; i < kMaxActiveFiles; ++i) {
            if (files_.Get(i) == file) {
                file->~File();
                files_.Free(i);
                count_--;
                return {};
            }
        }
        return std::unexpected(FdError::kInvalidArgument);
    }

    return {};
}

File *FileTable::Find(const vfs::Path &path)
{
    for (size_t i = 0; i < kMaxActiveFiles; ++i) {
        File *file = files_.Get(i);
        if (file != nullptr && file->HasRefs() && file->path == path) {
            return file;
        }
    }
    return nullptr;
}

const File *FileTable::Find(const vfs::Path &path) const
{
    for (size_t i = 0; i < kMaxActiveFiles; ++i) {
        const File *file = files_.Get(i);
        if (file != nullptr && file->HasRefs() && file->path == path) {
            return file;
        }
    }
    return nullptr;
}
// ============================================================================
// OpenFileTable Implementation
// ============================================================================

OpenFileTable::OpenFileTable() : open_count_(0) {}

OpenFileTable::~OpenFileTable()
{
    std::lock_guard lock(lock_);

    for (size_t i = 0; i < kMaxOpenFiles; ++i) {
        OpenFileEntry *entry = entries_.Get(i);
        if (entry != nullptr && entry->HasRefs()) {
            entry->handle = {};
            entry->~OpenFileEntry();
            entries_.Free(i);
        }
    }
}

FdResult<OpenFileEntry *> OpenFileTable::OpenFile(File *file, OpenMode flags)
{
    if (file == nullptr) {
        return std::unexpected(FdError::kInvalidArgument);
    }

    std::lock_guard lock(lock_);

    size_t idx = entries_.Allocate();
    if (idx == std::numeric_limits<size_t>::max()) {
        return std::unexpected(FdError::kIoError);
    }

    OpenFileEntry *entry = entries_.Get(idx);
    ASSERT_NOT_NULL(entry);

    new (entry) OpenFileEntry();

    entry->handle    = FileHandle::Wrap(file);
    entry->flags     = static_cast<u32>(flags);
    entry->offset    = 0;
    entry->is_append = HasMode(flags, OpenMode::kAppend);

    open_count_++;
    return entry;
}

FdResult<OpenFileEntry *> OpenFileTable::OpenPipe(IO::Pipe<kStdioBufferSize> &pipe)
{
    std::lock_guard lock(lock_);

    size_t idx = entries_.Allocate();
    if (idx == std::numeric_limits<size_t>::max()) {
        return std::unexpected(FdError::kIoError);
    }

    OpenFileEntry *entry = entries_.Get(idx);
    ASSERT_NOT_NULL(entry);

    new (entry) OpenFileEntry();

    entry->handle    = FileHandle::Wrap(&pipe);
    entry->flags     = static_cast<u32>(OpenMode::kReadWrite);
    entry->offset    = 0;
    entry->is_append = false;

    open_count_++;
    return entry;
}

FdResult<> OpenFileTable::CloseFile(OpenFileEntry *entry)
{
    if (entry == nullptr) {
        return std::unexpected(FdError::kInvalidArgument);
    }

    std::lock_guard lock(lock_);

    if (!entry->HasRefs()) {
        return std::unexpected(FdError::kAlreadyClosed);
    }

    u32 old_count = entry->GetRefCount();
    entry->Release();

    if (old_count == 1) {
        entry->handle = {};

        for (size_t i = 0; i < kMaxOpenFiles; ++i) {
            if (entries_.Get(i) == entry) {
                entry->~OpenFileEntry();
                entries_.Free(i);
                open_count_--;
                return {};
            }
        }
        return std::unexpected(FdError::kInvalidArgument);
    }

    return {};
}

FdResult<u64> OpenFileTable::GetOffset(const OpenFileEntry *entry) const
{
    if (entry == nullptr) {
        return std::unexpected(FdError::kInvalidArgument);
    }

    std::lock_guard lock(lock_);

    return entry->offset;
}

FdResult<> OpenFileTable::SetOffset(OpenFileEntry *entry, u64 offset) const
{
    if (entry == nullptr) {
        return std::unexpected(FdError::kInvalidArgument);
    }

    std::lock_guard lock(lock_);

    entry->offset = offset;
    return {};
}

// ============================================================================
// FdManager Implementation
// ============================================================================

FdManager::FdManager() = default;

FdManager::~FdManager() = default;

FdResult<fd_t> FdManager::Open(const vfs::Path &path, OpenMode flags)
{
    auto file_result = file_table_.GetOrCreate(path);
    RET_UNEXPECTED_IF_ERR(file_result);
    File *file = *file_result;

    auto open_result = open_file_table_.OpenFile(file, flags);
    if (!open_result) {
        return std::unexpected(open_result.error());
    }
    OpenFileEntry *global_entry = *open_result;

    FdTable *fd_table = GetCurrentProcessFdTable();
    if (fd_table == nullptr) {
        open_file_table_.CloseFile(global_entry);
        return std::unexpected(FdError::kIoError);
    }

    auto fd_result = fd_table->AllocateFd(global_entry);
    if (!fd_result) {
        open_file_table_.CloseFile(global_entry);
        return std::unexpected(fd_result.error());
    }

    return *fd_result;
}

FdResult<> FdManager::Close(fd_t fd)
{
    FdTable *fd_table = GetCurrentProcessFdTable();
    RET_UNEXPECTED_IF(fd_table == nullptr, FdError::kIoError);

    OpenFileEntry *entry = fd_table->GetEntry(fd);
    RET_UNEXPECTED_IF(!entry, FdError::kBadFileDescriptor);

    auto close_result = open_file_table_.CloseFile(entry);
    RET_UNEXPECTED_IF(!close_result, close_result.error());

    auto free_result = fd_table->FreeFd(fd);
    RET_UNEXPECTED_IF(!free_result, free_result.error());

    return {};
}

FdResult<size_t> FdManager::Read(fd_t fd, std::span<byte> buffer)
{
    RET_UNEXPECTED_IF(buffer.empty(), FdError::kInvalidArgument);

    FdTable *fd_table = GetCurrentProcessFdTable();
    ASSERT_NOT_NULL(fd_table);

    OpenFileEntry *entry = fd_table->GetEntry(fd);
    RET_UNEXPECTED_IF(!entry, FdError::kBadFileDescriptor);

    OpenMode mode = static_cast<OpenMode>(entry->flags);
    RET_UNEXPECTED_IF(!HasMode(mode, OpenMode::kRead), FdError::kPermissionDenied);

    if (entry->IsFile()) {
        File *file = entry->GetFile();
        RET_UNEXPECTED_IF(file == nullptr, FdError::kBadFileDescriptor);

        auto result = vfs::ReadFile(file->path, buffer.data(), buffer.size(), entry->offset);
        RET_UNEXPECTED_IF(!result, FdError::kIoError);

        entry->offset += *result;
        return *result;
    } else if (entry->IsPipe()) {
        auto *pipe = entry->GetPipe();
        RET_UNEXPECTED_IF(pipe == nullptr, FdError::kBadFileDescriptor);

        auto result = pipe->Read(buffer);
        RET_UNEXPECTED_IF(!result, FdError::kIoError);
        return *result;
    }

    return std::unexpected(FdError::kBadFileDescriptor);
}

FdResult<size_t> FdManager::Write(fd_t fd, std::span<const byte> buffer)
{
    RET_UNEXPECTED_IF(buffer.empty(), FdError::kInvalidArgument);

    FdTable *fd_table = GetCurrentProcessFdTable();
    ASSERT_NOT_NULL(fd_table);

    OpenFileEntry *entry = fd_table->GetEntry(fd);
    RET_UNEXPECTED_IF(!entry, FdError::kBadFileDescriptor);

    OpenMode mode = static_cast<OpenMode>(entry->flags);
    RET_UNEXPECTED_IF(!HasMode(mode, OpenMode::kWrite), FdError::kPermissionDenied);

    if (entry->IsFile()) {
        File *file = entry->GetFile();
        RET_UNEXPECTED_IF(file == nullptr, FdError::kBadFileDescriptor);

        if (entry->is_append) {
            entry->offset = file->size;
        }

        auto result = vfs::WriteFile(file->path, buffer.data(), buffer.size(), entry->offset);
        RET_UNEXPECTED_IF(!result, FdError::kIoError);

        entry->offset += *result;
        return *result;
    } else if (entry->IsPipe()) {
        auto *pipe = entry->GetPipe();
        RET_UNEXPECTED_IF(pipe == nullptr, FdError::kBadFileDescriptor);

        auto result = pipe->Write(buffer);
        RET_UNEXPECTED_IF(!result, FdError::kIoError);
        return *result;
    }

    return std::unexpected(FdError::kBadFileDescriptor);
}

FdResult<ssize_t> FdManager::Seek(fd_t fd, ssize_t offset, FdSeek whence)
{
    FdTable *fd_table = GetCurrentProcessFdTable();
    ASSERT_NOT_NULL(fd_table);

    OpenFileEntry *entry = fd_table->GetEntry(fd);
    RET_UNEXPECTED_IF(!entry, FdError::kBadFileDescriptor);

    RET_UNEXPECTED_IF(!entry->IsFile(), FdError::kInvalidArgument);

    File *file = entry->GetFile();
    RET_UNEXPECTED_IF(file == nullptr, FdError::kBadFileDescriptor);

    ssize_t new_offset = 0;
    switch (whence) {
        case FdSeek::kSet:
            new_offset = offset;
            break;
        case FdSeek::kCur:
            new_offset = entry->offset + offset;
            break;
        case FdSeek::kEnd:
            new_offset = file->size + offset;
            break;
        default:
            return std::unexpected(FdError::kInvalidArgument);
    }

    RET_UNEXPECTED_IF(
        new_offset > static_cast<ssize_t>(file->size) || new_offset < 0, FdError::kInvalidArgument
    );

    entry->offset = static_cast<u64>(new_offset);
    return new_offset;
}

FdTable *FdManager::GetCurrentProcessFdTable()
{
    auto process = SchedulingModule::Get().GetProcesses().GetCurrentProcess();
    if (!process) {
        return nullptr;
    }

    return process.value()->fd_table;
}

const FdTable *FdManager::GetCurrentProcessFdTable() const
{
    auto process = SchedulingModule::Get().GetProcesses().GetCurrentProcess();
    if (!process) {
        return nullptr;
    }

    return process.value()->fd_table;
}

}  // namespace Fs
