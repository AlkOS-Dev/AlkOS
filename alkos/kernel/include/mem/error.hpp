#ifndef ALKOS_KERNEL_INCLUDE_MEM_ERROR_HPP_
#define ALKOS_KERNEL_INCLUDE_MEM_ERROR_HPP_

namespace mem
{

enum class MemError {
    OutOfMemory,
    InvalidArgument,
    NotFound,
};

}  // namespace mem

#endif  // ALKOS_KERNEL_INCLUDE_MEM_ERROR_HPP_
