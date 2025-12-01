#ifndef KERNEL_SRC_IO_TYPES_HPP_
#define KERNEL_SRC_IO_TYPES_HPP_

#include "types.hpp"

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

}  // namespace IO

#endif /* KERNEL_SRC_IO_TYPES_HPP_ */
