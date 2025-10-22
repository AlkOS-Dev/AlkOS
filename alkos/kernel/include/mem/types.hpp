#ifndef ALKOS_KERNEL_INCLUDE_MEM_TYPES_HPP_
#define ALKOS_KERNEL_INCLUDE_MEM_TYPES_HPP_

#include <extensions/expected.hpp>
#include <extensions/type_traits.hpp>
#include <extensions/types.hpp>
#include "hal/constants.hpp"

template <typename T, typename E>
using Expected = std::expected<T, E>;

template <typename E>
using Unexpected = std::unexpected<E>;

#define EXPECTED_RET_IF_ERR(res) \
    if (!res)                    \
    return Unexpected(res.error())

namespace Mem
{

template <typename T>
using VirtualPtr = T *;

template <typename T>
using VPtr = VirtualPtr<T>;

template <typename T>
using PhysicalPtr = T *;

template <typename T>
using PPtr = PhysicalPtr<T>;

template <typename T>
VPtr<T> PhysToVirt(PPtr<T> physAddr)
{
    return reinterpret_cast<VPtr<T>>(
        reinterpret_cast<uintptr_t>(physAddr) + hal::kKernelVirtualAddressStart
    );
}

template <typename T>
PPtr<T> VirtToPhys(VPtr<T> virtAddr)
{
    return reinterpret_cast<PPtr<T>>(
        reinterpret_cast<uintptr_t>(virtAddr) - hal::kKernelVirtualAddressStart
    );
}

template <typename T>
uintptr_t PtrToUptr(T *ptr)
{
    return reinterpret_cast<uintptr_t>(ptr);
}

template <typename T>
T *UptrToPtr(uintptr_t uptr)
{
    return reinterpret_cast<T *>(uptr);
}

}  // namespace Mem

#endif  // ALKOS_KERNEL_INCLUDE_MEM_TYPES_HPP_
