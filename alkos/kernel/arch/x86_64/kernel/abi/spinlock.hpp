#ifndef ALKOS_KERNEL_ARCH_X86_64_KERNEL_ABI_SPINLOCK_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_KERNEL_ABI_SPINLOCK_HPP_

#include "spinlock.hpp"

#include <constants.hpp>
#include <extensions/types.hpp>

namespace arch
{
class alignas(kCacheLineSizeBytes) Spinlock : public SpinlockAbi
{
    FAST_CALL void Pause() { asm volatile("pause"); }

    public:
    /* basic operations */
    FORCE_INLINE_F void lock()
    {
        while (__builtin_expect(__sync_lock_test_and_set(&lock_, 1), 0)) {
            Pause();
        }
    }

    FORCE_INLINE_F void unlock() { __sync_lock_release(&lock_); }

    FORCE_INLINE_F NODISCARD bool try_lock() { return !__sync_lock_test_and_set(&lock_, 1); }

    protected:
    u32 lock_ = 0;
};
}  // namespace arch

#endif  // ALKOS_KERNEL_ARCH_X86_64_KERNEL_ABI_SPINLOCK_HPP_
