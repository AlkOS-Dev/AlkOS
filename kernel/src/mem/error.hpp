#ifndef KERNEL_SRC_MEM_ERROR_HPP_
#define KERNEL_SRC_MEM_ERROR_HPP_

namespace Mem
{

enum class MemError {
    OutOfMemory,
    InvalidArgument,
    NotFound,
};

}  // namespace Mem

#endif  // KERNEL_SRC_MEM_ERROR_HPP_
