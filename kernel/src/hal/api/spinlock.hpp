#ifndef KERNEL_SRC_HAL_API_SPINLOCK_HPP_
#define KERNEL_SRC_HAL_API_SPINLOCK_HPP_

#include <defines.hpp>

namespace arch
{
class Spinlock;

struct SpinlockAPI {
    void Lock();
    void Unlock();
    bool TryLock();
    NODISCARD bool IsLocked() const;
};

}  // namespace arch

#endif  // KERNEL_SRC_HAL_API_SPINLOCK_HPP_
