#ifndef ALKOS_KERNEL_INCLUDE_MEM_VIRT_PTR_HPP_
#define ALKOS_KERNEL_INCLUDE_MEM_VIRT_PTR_HPP_

#include <extensions/type_traits.hpp>
#include <extensions/types.hpp>
#include "hal/constants.hpp"

namespace Mem
{

template <typename T>
using VirtualPtr = T *;
template <typename T>
using VPtr = VirtualPtr<T>;

}  // namespace Mem

#endif  // ALKOS_KERNEL_INCLUDE_MEM_VIRT_PTR_HPP_
