#ifndef ALKOS_KERNEL_INCLUDE_HAL_API_SPINLOCK_HPP_
#define ALKOS_KERNEL_INCLUDE_HAL_API_SPINLOCK_HPP_

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

#endif  // ALKOS_KERNEL_INCLUDE_HAL_API_SPINLOCK_HPP_
