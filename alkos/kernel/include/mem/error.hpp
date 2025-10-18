#ifndef ALKOS_KERNEL_INCLUDE_MEM_ERROR_HPP_
#define ALKOS_KERNEL_INCLUDE_MEM_ERROR_HPP_

namespace Mem
{

enum class MemError {
    OutOfMemory,
    InvalidArgument,
    NotFound,
};

}  // namespace Mem

#endif  // ALKOS_KERNEL_INCLUDE_MEM_ERROR_HPP_
