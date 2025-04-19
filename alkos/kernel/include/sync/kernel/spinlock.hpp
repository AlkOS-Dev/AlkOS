#ifndef ALKOS_KERNEL_INCLUDE_SYNC_KERNEL_SPINLOCK_HPP_
#define ALKOS_KERNEL_INCLUDE_SYNC_KERNEL_SPINLOCK_HPP_

#include <spinlock.hpp>

template <bool kDebugCapabilities = false>
class BaseSpinlock : public arch::Spinlock
{
    /* TODO: implement here conditional debug capabilities */

    public:
    FORCE_INLINE_F void lock() { Spinlock::lock(); }

    FORCE_INLINE_F void unlock() { Spinlock::unlock(); }

    NODISCARD FORCE_INLINE_F bool try_lock() { return Spinlock::try_lock(); }
};

using Spinlock = BaseSpinlock<kIsDebugBuild>;

#endif  // ALKOS_KERNEL_INCLUDE_SYNC_KERNEL_SPINLOCK_HPP_
