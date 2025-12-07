#ifndef KERNEL_SRC_IO_PIPE_HPP_
#define KERNEL_SRC_IO_PIPE_HPP_

#include <data_structures/atomic_cyclic_buffer.hpp>
#include <hal/spinlock.hpp>

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

        if (bytes_written == 0 && buffer.size() > 0) {
            return std::unexpected(Error::Retry);
        }
        return bytes_written;
    }

    IoResult Read(std::span<byte> buffer) override
    {
        size_t bytes_read = buffer_.Read(buffer);

        if (bytes_read == 0 && buffer.size() > 0) {
            return std::unexpected(Error::Retry);
        }
        return bytes_read;
    }

    private:
    data_structures::AtomicCyclicBuffer<byte, Size> buffer_;
};

}  // namespace IO

#endif  // KERNEL_SRC_IO_PIPE_HPP_
