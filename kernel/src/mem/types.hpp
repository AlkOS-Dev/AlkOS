#ifndef KERNEL_SRC_MEM_TYPES_HPP_
#define KERNEL_SRC_MEM_TYPES_HPP_

#include <expected.hpp>
#include <type_traits.hpp>
#include <types.hpp>
#include "hal/constants.hpp"

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
        reinterpret_cast<uintptr_t>(physAddr) + hal::kDirectMapAddrStart
    );
}

template <typename T>
PPtr<T> VirtToPhys(VPtr<T> virtAddr)
{
    uintptr_t vaddr = reinterpret_cast<uintptr_t>(virtAddr);
    if (vaddr >= hal::kDirectMapAddrStart) {
        return reinterpret_cast<PPtr<T>>(vaddr - hal::kDirectMapAddrStart);
    }

    return reinterpret_cast<PPtr<T>>(vaddr - hal::kKernelVirtualAddressStart);
}

template <typename T>
uptr PtrToUptr(T *ptr)
{
    return reinterpret_cast<uptr>(ptr);
}

template <typename T>
T *UptrToPtr(uptr uptr)
{
    return reinterpret_cast<T *>(uptr);
}

}  // namespace Mem

#endif  // KERNEL_SRC_MEM_TYPES_HPP_
