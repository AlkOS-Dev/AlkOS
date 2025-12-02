#ifndef KERNEL_SRC_IO_ERROR_HPP_
#define KERNEL_SRC_IO_ERROR_HPP_

namespace Io
{

enum class Error {
    None,
    Retry,        // Resource busy/buffer full (Non-blocking)
    EndOfFile,    // Connection closed
    DeviceError,  // Hardware fault
    InvalidInput
};

}

#endif /* KERNEL_SRC_IO_ERROR_HPP_ */
