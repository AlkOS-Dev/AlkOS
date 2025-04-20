#ifndef ALKOS_KERNEL_INCLUDE_SYNC_KERNEL_SPINLOCK_HPP_
#define ALKOS_KERNEL_INCLUDE_SYNC_KERNEL_SPINLOCK_HPP_

#include <spinlock.hpp>

struct Spinlock : arch::Spinlock {
    // ------------------------------
    // Binding for CXX lib
    // ------------------------------

    FORCE_INLINE_F void lock() { arch::Spinlock::Lock(); }
    FORCE_INLINE_F void unlock() { arch::Spinlock::Unlock(); }
};

#endif  // ALKOS_KERNEL_INCLUDE_SYNC_KERNEL_SPINLOCK_HPP_
