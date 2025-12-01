#ifndef KERNEL_SRC_IO_MULTIPLEXER_HPP_
#define KERNEL_SRC_IO_MULTIPLEXER_HPP_

#include <array.hpp>
#include <hal/spinlock.hpp>
#include <io/stream.hpp>
#include "io/error.hpp"

namespace IO
{

using std::expected;
using std::unexpected;

/**
 * @brief A writer that broadcasts data to multiple other writers.
 * * @tparam MaxOutputs The maximum number of attached writers (static allocation).
 */
template <size_t MaxOutputs>
class MultiplexerWriter : public IWriter
{
    public:
    MultiplexerWriter()          = default;
    virtual ~MultiplexerWriter() = default;

    /**
     * @brief Attaches a new writer to the multiplexer.
     */
    expected<void> AddOutput(IWriter &writer)
    {
        std::lock_guard guard(lock_);
        if (count_ >= MaxOutputs) {
            return unexpected(Error::InvalidInput);
        }
        outputs_[count_++] = &writer;
        return {};
    }

    /**
     * @brief Broadcasts the buffer to all attached writers.
     */
    IoResult Write(std::span<const byte> buffer) override
    {
        std::lock_guard guard(lock_);

        for (size_t i = 0; i < count_; ++i) {
            if (outputs_[i]) {
                // Even if one fails, we try the others.
                (void)outputs_[i]->Write(buffer);
            }
        }

        // TODO: Consider a better way
        // From the perspective of the caller, the data was "handled".
        return buffer.size();
    }

    private:
    std::array<IWriter *, MaxOutputs> outputs_{nullptr};
    size_t count_ = 0;

    hal::Spinlock lock_;
};

}  // namespace IO

#endif  // KERNEL_SRC_IO_MULTIPLEXER_HPP_
