#ifndef KERNEL_SRC_SYNC_SPINLOCK_HPP_
#define KERNEL_SRC_SYNC_SPINLOCK_HPP_

#include <hal/spinlock.hpp>

struct Spinlock : hal::Spinlock {
    // ------------------------------
    // Binding for CXX lib
    // ------------------------------

    FORCE_INLINE_F void lock() { hal::Spinlock::Lock(); }
    FORCE_INLINE_F void unlock() { hal::Spinlock::Unlock(); }
    FORCE_INLINE_F bool try_lock() { return hal::Spinlock::TryLock(); }
};

#endif  // KERNEL_SRC_SYNC_SPINLOCK_HPP_
