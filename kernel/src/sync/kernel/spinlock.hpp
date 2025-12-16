#ifndef KERNEL_SRC_SYNC_KERNEL_SPINLOCK_HPP_
#define KERNEL_SRC_SYNC_KERNEL_SPINLOCK_HPP_

#include <hal/spinlock.hpp>

struct Spinlock : arch::Spinlock {
    // ------------------------------
    // Binding for CXX lib
    // ------------------------------

    FORCE_INLINE_F void lock() { hal::Spinlock::Lock(); }
    FORCE_INLINE_F void unlock() { hal::Spinlock::Unlock(); }
};

#endif  // KERNEL_SRC_SYNC_KERNEL_SPINLOCK_HPP_
