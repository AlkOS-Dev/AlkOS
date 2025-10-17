#ifndef ALKOS_KERNEL_INCLUDE_MEM_HEAP_HPP_
#define ALKOS_KERNEL_INCLUDE_MEM_HEAP_HPP_

#include <extensions/expected.hpp>

#include "mem/error.hpp"
#include "mem/virt/ptr.hpp"

namespace mem
{
template <typename T, typename E>
using Expected = std::expected<T, E>;
template <typename E>
using Unexpected = std::unexpected<E>;

Expected<VirtualPtr<void>, MemError> KMalloc(size_t);
Expected<void, MemError> KFree(VirtualPtr<void>);
}  // namespace mem

#endif  // ALKOS_KERNEL_INCLUDE_MEM_HEAP_HPP_
