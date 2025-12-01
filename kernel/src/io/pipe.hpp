#ifndef KERNEL_SRC_IO_PIPE_HPP_
#define KERNEL_SRC_IO_PIPE_HPP_

#include <algorithm.hpp>
#include <hal/spinlock.hpp>
#include <memory.hpp>

#include "io/stream.hpp"

namespace IO
{

template <size_t Size>
class Pipe : public IStream
{
    public:
    Pipe()          = default;
    virtual ~Pipe() = default;

    IoResult Write(std::span<const byte> buffer) override
    {
        std::lock_guard guard(lock_);

        size_t bytes_written = 0;
        for (const auto &b : buffer) {
            if (IsFull()) {
                // Caller decides if they want to retry (spin) or yield.
                break;
            }
            buffer_[head_] = b;
            head_          = (head_ + 1) % Size;
            count_++;
            bytes_written++;
        }

        if (bytes_written == 0 && buffer.size() > 0) {
            return std::unexpected(Error::Retry);
        }
        return bytes_written;
    }

    IoResult Read(std::span<byte> buffer) override
    {
        std::lock_guard guard(lock_);

        size_t bytes_read = 0;
        for (size_t i = 0; i < buffer.size(); ++i) {
            if (IsEmpty()) {
                break;
            }
            buffer[i] = buffer_[tail_];
            tail_     = (tail_ + 1) % Size;
            count_--;
            bytes_read++;
        }

        if (bytes_read == 0 && buffer.size() > 0) {
            return std::unexpected(Error::Retry);
        }
        return bytes_read;
    }

    private:
    bool IsFull() const { return count_ == Size; }
    bool IsEmpty() const { return count_ == 0; }

    // Ring Buffer State
    byte buffer_[Size];
    size_t head_  = 0;  // Write index
    size_t tail_  = 0;  // Read index
    size_t count_ = 0;

    hal::Spinlock lock_;
};

}  // namespace IO

#endif  // KERNEL_SRC_IO_PIPE_HPP_
