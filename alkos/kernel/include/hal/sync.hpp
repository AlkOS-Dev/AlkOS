#ifndef ALKOS_KERNEL_INCLUDE_HAL_SYNC_HPP_
#define ALKOS_KERNEL_INCLUDE_HAL_SYNC_HPP_

#include <hal/impl/sync.hpp>

namespace hal
{
WRAP_CALL void FullMemFence() { arch::FullMemFence(); }

WRAP_CALL void LoadMemFence() { arch::LoadMemFence(); }

WRAP_CALL void SaveMemFence() { arch::SaveMemFence(); }

template <AtomicT T>
WRAP_CALL T AtomicLoad(volatile const T *ptr);

template <AtomicT T>
WRAP_CALL void AtomicStore(volatile T *ptr, typename T::BaseT value);

template <AtomicT T>
WRAP_CALL T
AtomicCompareExchange(volatile T *ptr, typename T::BaseT expected, typename T::BaseT desired);

template <AtomicT T>
WRAP_CALL T AtomicExchange(volatile T *ptr, typename T::BaseT value);

template <AtomicT T>
WRAP_CALL T AtomicAdd(volatile T *ptr, typename T::BaseT value);

template <AtomicT T>
WRAP_CALL T AtomicSub(volatile T *ptr, typename T::BaseT value);

template <AtomicT T>
WRAP_CALL T AtomicIncrement(volatile T *ptr);

template <AtomicT T>
WRAP_CALL T AtomicDecrement(volatile T *ptr);

}  // namespace hal

#endif  // ALKOS_KERNEL_INCLUDE_HAL_SYNC_HPP_
