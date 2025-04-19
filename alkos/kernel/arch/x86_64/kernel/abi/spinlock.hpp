#ifndef ALKOS_KERNEL_ARCH_X86_64_KERNEL_ABI_SPINLOCK_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_KERNEL_ABI_SPINLOCK_HPP_

#include "spinlock.hpp"

#include <stdatomic.h>
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
        while (atomic_flag_test_and_set_explicit(&lock_, memory_order_acquire)) {
            Pause();
        }
    }

    FORCE_INLINE_F void unlock() { atomic_flag_clear_explicit(&lock_, memory_order_release); }

    FORCE_INLINE_F NODISCARD bool try_lock()
    {
        return !atomic_flag_test_and_set_explicit(&lock_, memory_order_acquire);
    }

    /* debug capabilities */
    NODISCARD FORCE_INLINE_F bool is_locked() const { return locked_; }

    FORCE_INLINE_F void set_locked(const bool locked) { locked_ = locked; }

    NODISCARD FORCE_INLINE_F u32 get_owner_id() const { return owner_id_; }

    FORCE_INLINE_F void set_owner_id(const u32 id) { owner_id_ = id; }

    protected:
    alignas(u32) atomic_flag lock_ = ATOMIC_FLAG_INIT;
    bool locked_ : 1;
    u32 owner_id_;
};
}  // namespace arch

#endif  // ALKOS_KERNEL_ARCH_X86_64_KERNEL_ABI_SPINLOCK_HPP_
