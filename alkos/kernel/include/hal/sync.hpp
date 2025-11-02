#ifndef ALKOS_KERNEL_INCLUDE_HAL_SYNC_HPP_
#define ALKOS_KERNEL_INCLUDE_HAL_SYNC_HPP_

#include <hal/impl/sync.hpp>

namespace hal
{
WRAP_CALL void FullMemFence() { arch::FullMemFence(); }

WRAP_CALL void LoadMemFence() { arch::LoadMemFence(); }

WRAP_CALL void SaveMemFence() { arch::SaveMemFence(); }

template <AtomicT T>
WRAP_CALL typename T::BaseT AtomicLoad(volatile const T *ptr)
{
    return arch::AtomicLoad(ptr);
}

template <AtomicT T>
WRAP_CALL void AtomicStore(volatile T *ptr, typename T::BaseT value)
{
    arch::AtomicStore(ptr, value);
}

template <AtomicT T>
WRAP_CALL typename T::BaseT AtomicCompareExchange(
    volatile T *ptr, typename T::BaseT expected, typename T::BaseT desired
)
{
    return arch::AtomicCompareExchange(ptr, expected, desired);
}
template <AtomicT T>
WRAP_CALL typename T::BaseT AtomicExchange(volatile T *ptr, typename T::BaseT value)
{
    return arch::AtomicExchange(ptr, value);
}

template <AtomicT T>
WRAP_CALL typename T::BaseT AtomicAdd(volatile T *ptr, typename T::BaseT value)
{
    return arch::AtomicAdd(ptr, value);
}

template <AtomicT T>
WRAP_CALL typename T::BaseT AtomicSub(volatile T *ptr, typename T::BaseT value)
{
    return arch::AtomicSub(ptr, value);
}

template <AtomicT T>
WRAP_CALL typename T::BaseT AtomicIncrement(volatile T *ptr)
{
    return arch::AtomicIncrement(ptr);
}

template <AtomicT T>
WRAP_CALL typename T::BaseT AtomicDecrement(volatile T *ptr)
{
    return arch::AtomicDecrement(ptr);
}

}  // namespace hal

#endif  // ALKOS_KERNEL_INCLUDE_HAL_SYNC_HPP_
