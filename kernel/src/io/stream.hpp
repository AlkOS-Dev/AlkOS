#ifndef KERNEL_SRC_IO_STREAM_HPP_
#define KERNEL_SRC_IO_STREAM_HPP_

#include <expected.hpp>
#include <span.hpp>

#include "io/error.hpp"
#include "mem/types.hpp"

namespace IO
{

using IoResult = Expected<size_t, Error>;

/**
 * @brief Abstract interface for reading bytes.
 */
class IReader
{
    public:
    virtual ~IReader() = default;

    /// Read bytes into the provided buffer.
    /// Returns the actual number of bytes read.
    virtual IoResult Read(std::span<byte> buffer) = 0;

    /// Helper for single char (convenience)
    /// Returns error if buffer empty
    virtual Expected<char, Error> GetChar()
    {
        byte c;
        auto res = Read(std::span<byte>(&c, 1));
        if (!res) {
            return std::unexpected(res.error());
        }
        if (*res == 0) {
            return Unexpected(Error::Retry);
        }
        return static_cast<char>(c);
    }
};

/**
 * @brief Abstract interface for writing bytes.
 */
class IWriter
{
    public:
    virtual ~IWriter() = default;

    // Write bytes from the provided buffer.
    // Returns the actual number of bytes written.
    virtual IoResult Write(std::span<const byte> buffer) = 0;

    // Helper for single char
    virtual IoResult PutChar(char c)
    {
        const byte b = static_cast<byte>(c);
        return Write(std::span<const byte>(&b, 1));
    }

    // Helper for strings
    virtual IoResult WriteString(const char *str)
    {
        size_t len    = 0;
        const char *s = str;
        while (*s++ != 0) {
            len++;
        }
        return Write(std::span<const byte>(reinterpret_cast<const byte *>(str), len));
    }
};

/**
 * @brief A Stream is a bidirectional device (e.g., Serial Port, TCP Socket).
 */
class IStream : public IReader, public IWriter
{
    public:
    virtual ~IStream() = default;
};

}  // namespace IO

#endif  // KERNEL_SRC_IO_STREAM_HPP_
