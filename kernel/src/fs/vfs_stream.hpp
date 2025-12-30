#ifndef KERNEL_SRC_FS_VFS_STREAM_HPP_
#define KERNEL_SRC_FS_VFS_STREAM_HPP_

#include <data_structures/array_structures.hpp>
#include <span.hpp>
#include <string.hpp>

#include <io/stream.hpp>
#include <vfs/path.hpp>

namespace Fs
{

/**
 * @brief VfsStream wraps VFS operations to implement IStream interface
 * This allows the file descriptor subsystem to use VFS for I/O operations
 */
class VfsStream : public IO::IStream
{
    public:
    /**
     * @brief Construct a VfsStream for a given path
     * @param path The file path in the VFS
     */
    explicit VfsStream(const char *path);

    ~VfsStream() override = default;

    // IReader implementation
    IO::IoResult Read(std::span<byte> buffer) override;

    // IWriter implementation
    IO::IoResult Write(std::span<const byte> buffer) override;

    /**
     * @brief Get the path associated with this stream
     * @return The file path as a C string
     */
    const char *GetPath() const { return path_.CString(); }

    private:
    vfs::Path path_;
    u64 current_offset_;
};

/**
 * @brief StdinStream wraps stdin for reading from debug terminal
 */
class StdinStream : public IO::IReader
{
    public:
    StdinStream()           = default;
    ~StdinStream() override = default;

    IO::IoResult Read(std::span<byte> buffer) override;
};

/**
 * @brief StdoutStream wraps stdout for writing to debug terminal
 */
class StdoutStream : public IO::IWriter
{
    public:
    StdoutStream()           = default;
    ~StdoutStream() override = default;

    IO::IoResult Write(std::span<const byte> buffer) override;
};

/**
 * @brief StderrStream wraps stderr for writing to debug terminal
 */
class StderrStream : public IO::IWriter
{
    public:
    StderrStream()           = default;
    ~StderrStream() override = default;

    IO::IoResult Write(std::span<const byte> buffer) override;
};

// ============================================================================
// Standard Stream Wrappers (implement IStream for use with File::stream)
// ============================================================================

/**
 * @brief Wrapper for StdinStream that implements IStream interface
 * This allows StdinStream to be used with File::stream (IO::IStream*)
 */
class StdinStreamWrapper : public IO::IStream
{
    public:
    StdinStreamWrapper()           = default;
    ~StdinStreamWrapper() override = default;

    // IReader implementation
    IO::IoResult Read(std::span<byte> buffer) override { return stdin_stream_.Read(buffer); }

    // IWriter implementation (not supported for stdin)
    IO::IoResult Write(std::span<const byte> buffer) override
    {
        (void)buffer;
        return std::unexpected(IO::Error::InvalidInput);
    }

    private:
    StdinStream stdin_stream_;
};

/**
 * @brief Wrapper for StdoutStream that implements IStream interface
 * This allows StdoutStream to be used with File::stream (IO::IStream*)
 */
class StdoutStreamWrapper : public IO::IStream
{
    public:
    StdoutStreamWrapper()           = default;
    ~StdoutStreamWrapper() override = default;

    // IReader implementation (not supported for stdout)
    IO::IoResult Read(std::span<byte> buffer) override
    {
        (void)buffer;
        return std::unexpected(IO::Error::InvalidInput);
    }

    // IWriter implementation
    IO::IoResult Write(std::span<const byte> buffer) override
    {
        return stdout_stream_.Write(buffer);
    }

    private:
    StdoutStream stdout_stream_;
};

/**
 * @brief Wrapper for StderrStream that implements IStream interface
 * This allows StderrStream to be used with File::stream (IO::IStream*)
 */
class StderrStreamWrapper : public IO::IStream
{
    public:
    StderrStreamWrapper()           = default;
    ~StderrStreamWrapper() override = default;

    // IReader implementation (not supported for stderr)
    IO::IoResult Read(std::span<byte> buffer) override
    {
        (void)buffer;
        return std::unexpected(IO::Error::InvalidInput);
    }

    // IWriter implementation
    IO::IoResult Write(std::span<const byte> buffer) override
    {
        return stderr_stream_.Write(buffer);
    }

    private:
    StderrStream stderr_stream_;
};

}  // namespace Fs

#endif  // KERNEL_SRC_FS_VFS_STREAM_HPP_
