// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef USERSPACE_PROGRAMS_SHELL_STREAM_HPP_
#define USERSPACE_PROGRAMS_SHELL_STREAM_HPP_

#include <expected.hpp>
#include <span.hpp>

#include "internal/macros.hpp"

namespace IO
{

enum class Error {
    None,
    Retry,        // Resource busy/buffer full (Non-blocking)
    EndOfFile,    // Connection closed
    DeviceError,  // Hardware fault
    InvalidInput
};

using std::expected;
using std::unexpected;

using IoResult = expected<size_t, Error>;

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

}  // namespace IO

#endif  // USERSPACE_PROGRAMS_SHELL_STREAM_HPP_
