#ifndef ALKOS_KERNEL_HAL_SPINLOCK_HPP_
#define ALKOS_KERNEL_HAL_SPINLOCK_HPP_

namespace arch
{
/* Should be defined by architecture */
class Spinlock;

/* Defined by architecture to allow various optimizations */
struct SpinlockAbi {
    /* basic operations */
    void Lock();
    void Unlock();
    bool TryLock();
    NODISCARD bool IsLocked() const;
};

}  // namespace arch

/* Load architecture definition of component */
#include <hal/spinlock.hpp>

#endif  // ALKOS_KERNEL_HAL_SPINLOCK_HPP_
