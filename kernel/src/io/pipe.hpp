// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_SRC_IO_PIPE_HPP_
#define KERNEL_SRC_IO_PIPE_HPP_

#include <data_structures/atomic_cyclic_buffer.hpp>

#include "internal/macros.hpp"
#include "io/stream.hpp"

namespace IO
{

/// Assumes Single Producer - Single Consumer
/// Because it uses atomics and not locks
///
/// under the hood, it's IRQ safe
/// (Can connect eg. a Keyboard IRQ as a writer)
template <size_t Size>
class Pipe : public IStream
{
    public:
    Pipe()          = default;
    virtual ~Pipe() = default;

    IoResult Write(std::span<const byte> buffer) override
    {
        size_t bytes_written = buffer_.Write(buffer);

        RET_UNEXPECTED_IF(bytes_written == 0 && buffer.size() > 0, Error::Retry);
        return bytes_written;
    }

    IoResult Read(std::span<byte> buffer) override
    {
        size_t bytes_read = buffer_.Read(buffer);

        RET_UNEXPECTED_IF(bytes_read == 0 && buffer.size() > 0, Error::Retry);
        return bytes_read;
    }

    private:
    data_structures::AtomicCyclicBuffer<byte, Size> buffer_;
};

}  // namespace IO

#endif  // KERNEL_SRC_IO_PIPE_HPP_
