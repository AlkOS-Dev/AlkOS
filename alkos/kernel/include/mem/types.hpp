#ifndef ALKOS_KERNEL_INCLUDE_MEM_TYPES_HPP_
#define ALKOS_KERNEL_INCLUDE_MEM_TYPES_HPP_

#include <extensions/expected.hpp>

namespace Mem
{

template <typename T, typename E>
using Expected = std::expected<T, E>;

template <typename E>
using Unexpected = std::unexpected<E>;

}  // namespace Mem

#endif  // ALKOS_KERNEL_INCLUDE_MEM_TYPES_HPP_
