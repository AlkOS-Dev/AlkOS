#ifndef ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_SYNC_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_SYNC_HPP_

#include <extensions/defines.hpp>
#include <hal/api/sync.hpp>

namespace arch
{
FAST_CALL void FullMemFence() { __asm__ volatile("mfence" ::: "memory"); }

FAST_CALL void LoadMemFence() { __asm__ volatile("lfence" ::: "memory"); }

FAST_CALL void SaveMemFence() { __asm__ volatile("" ::: "memory"); }

template <hal::AtomicT T>
FAST_CALL typename T::BaseT AtomicLoad(volatile const T *ptr)
{
    return __atomic_load_n(&ptr->value, __ATOMIC_SEQ_CST);
}

template <hal::AtomicT T>
FAST_CALL void AtomicStore(volatile T *ptr, typename T::BaseT value)
{
    __atomic_store_n(&ptr->value, value, __ATOMIC_SEQ_CST);
}

template <hal::AtomicT T>
FAST_CALL typename T::BaseT AtomicCompareExchange(
    volatile T *ptr, typename T::BaseT expected, typename T::BaseT desired
)
{
    __atomic_compare_exchange_n(
        &ptr->value, &expected, desired, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST
    );
    return expected;
}

template <hal::AtomicT T>
FAST_CALL typename T::BaseT AtomicExchange(volatile T *ptr, typename T::BaseT value)
{
    return __atomic_exchange_n(&ptr->value, value, __ATOMIC_SEQ_CST);
}

template <hal::AtomicT T>
FAST_CALL typename T::BaseT AtomicAdd(volatile T *ptr, typename T::BaseT value)
{
    return __atomic_add_fetch(&ptr->value, value, __ATOMIC_SEQ_CST);
}

template <hal::AtomicT T>
FAST_CALL typename T::BaseT AtomicSub(volatile T *ptr, typename T::BaseT value)
{
    return __atomic_sub_fetch(&ptr->value, value, __ATOMIC_SEQ_CST);
}

template <hal::AtomicT T>
FAST_CALL typename T::BaseT AtomicIncrement(volatile T *ptr)
{
    return __atomic_add_fetch(&ptr->value, 1, __ATOMIC_SEQ_CST);
}

template <hal::AtomicT T>
FAST_CALL typename T::BaseT AtomicDecrement(volatile T *ptr)
{
    return __atomic_sub_fetch(&ptr->value, 1, __ATOMIC_SEQ_CST);
}
}  // namespace arch

#endif  // ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_SYNC_HPP_
