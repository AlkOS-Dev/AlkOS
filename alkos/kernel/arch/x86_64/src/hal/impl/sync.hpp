#ifndef ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_SYNC_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_SYNC_HPP_

#include <extensions/defines.hpp>

namespace arch
{
FAST_CALL void FullMemFence() { __asm__ volatile("mfence" ::: "memory"); }

FAST_CALL void LoadMemFence() { __asm__ volatile("lfence" ::: "memory"); }

FAST_CALL void SaveMemFence() { __asm__ volatile("" ::: "memory"); }

template <hal::AtomicT T>
FAST_CALL typename T::BaseT AtomicLoad(volatile const T *ptr)
{
    // __ATOMIC_SEQ_CST to najsilniejsze zamówienie pamięci.
    // Equivalent to std::atomic_load_explicit(ptr->value, std::memory_order_seq_cst)
    return __atomic_load_n(&ptr->value, __ATOMIC_SEQ_CST);
}

// AtomicStore
template <hal::AtomicT T>
FAST_CALL void AtomicStore(volatile T *ptr, typename T::BaseT value)
{
    // __ATOMIC_SEQ_CST to najsilniejsze zamówienie pamięci.
    // Equivalent to std::atomic_store_explicit(ptr->value, value, std::memory_order_seq_cst)
    __atomic_store_n(&ptr->value, value, __ATOMIC_SEQ_CST);
}

// AtomicCompareExchange
template <hal::AtomicT T>
FAST_CALL typename T::BaseT AtomicCompareExchange(
    volatile T *ptr, typename T::BaseT expected, typename T::BaseT desired
)
{
    // __atomic_compare_exchange_n zwraca bool (true jeśli wymiana nastąpiła).
    // Musimy zwrócić starą wartość, więc przechwytujemy 'expected'.
    // __ATOMIC_SEQ_CST for success, __ATOMIC_SEQ_CST for failure
    __atomic_compare_exchange_n(
        &ptr->value, &expected, desired,
        false,  // weak, try true for strong if needed
        __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST
    );
    return expected;  // 'expected' zostanie zaktualizowane do rzeczywistej wartości, jeśli operacja
                      // się nie powiodła
}

// AtomicExchange
template <hal::AtomicT T>
FAST_CALL typename T::BaseT AtomicExchange(volatile T *ptr, typename T::BaseT value)
{
    // Equivalent to std::atomic_exchange_explicit(ptr->value, value, std::memory_order_seq_cst)
    return __atomic_exchange_n(&ptr->value, value, __ATOMIC_SEQ_CST);
}

// AtomicAdd - zwraca NOWĄ wartość po operacji (zgodnie z konwencją niektórych Atomic API)
template <hal::AtomicT T>
FAST_CALL typename T::BaseT AtomicAdd(volatile T *ptr, typename T::BaseT value)
{
    // __atomic_add_fetch zwraca NOWĄ wartość
    // Equivalent to std::atomic_fetch_add_explicit(ptr->value, value, std::memory_order_seq_cst) +
    // value
    return __atomic_add_fetch(&ptr->value, value, __ATOMIC_SEQ_CST);
}

// AtomicSub - zwraca NOWĄ wartość po operacji
template <hal::AtomicT T>
FAST_CALL typename T::BaseT AtomicSub(volatile T *ptr, typename T::BaseT value)
{
    // __atomic_sub_fetch zwraca NOWĄ wartość
    return __atomic_sub_fetch(&ptr->value, value, __ATOMIC_SEQ_CST);
}

// AtomicIncrement - zwraca NOWĄ wartość po operacji
template <hal::AtomicT T>
FAST_CALL typename T::BaseT AtomicIncrement(volatile T *ptr)
{
    // __atomic_add_fetch z 1 zwraca NOWĄ wartość
    return __atomic_add_fetch(&ptr->value, 1, __ATOMIC_SEQ_CST);
}

// AtomicDecrement - zwraca NOWĄ wartość po operacji
template <hal::AtomicT T>
FAST_CALL typename T::BaseT AtomicDecrement(volatile T *ptr)
{
    // __atomic_sub_fetch z 1 zwraca NOWĄ wartość
    return __atomic_sub_fetch(&ptr->value, 1, __ATOMIC_SEQ_CST);
}
}  // namespace arch

#endif  // ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_SYNC_HPP_
