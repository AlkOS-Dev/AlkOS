#include "vfs_stream.hpp"

#include <expected.hpp>

#include "hal/debug_terminal.hpp"
#include "hal/panic.hpp"
#include "vfs.hpp"

namespace Fs
{

VfsStream::VfsStream(const char *path) : path_(path), current_offset_(0) {}

IO::IoResult VfsStream::Read(std::span<byte> buffer)
{
    // Read from VFS at current offset
    auto result = vfs::ReadFile(path_, buffer.data(), buffer.size(), current_offset_);

    if (!result) {
        // Convert VFS error to IO error
        return std::unexpected(IO::Error::DeviceError);
    }

    // Update offset
    current_offset_ += *result;

    return *result;
}

IO::IoResult VfsStream::Write(std::span<const byte> buffer)
{
    // Write to VFS at current offset
    auto result = vfs::WriteFile(path_, buffer.data(), buffer.size(), current_offset_);

    if (!result) {
        // Convert VFS error to IO error
        return std::unexpected(IO::Error::DeviceError);
    }

    // Update offset
    current_offset_ += *result;

    return *result;
}

IO::IoResult StdinStream::Read(std::span<byte> buffer)
{
    if (buffer.empty()) {
        return 0;
    }

    // Read from debug terminal (stdin)
    size_t chars_read =
        hal::DebugTerminalReadLine(reinterpret_cast<char *>(buffer.data()), buffer.size());

    return chars_read;
}

IO::IoResult StdoutStream::Write(std::span<const byte> buffer)
{
    if (buffer.empty()) {
        return 0;
    }

    // Write to debug terminal (stdout)
    hal::DebugTerminalWrite(reinterpret_cast<const char *>(buffer.data()));

    return buffer.size();
}

IO::IoResult StderrStream::Write(std::span<const byte> buffer)
{
    if (buffer.empty()) {
        return 0;
    }

    // Write to debug terminal (stderr)
    hal::DebugTerminalWrite(reinterpret_cast<const char *>(buffer.data()));

    return buffer.size();
}

}  // namespace Fs
