#ifndef ALKOS_KERNEL_ARCH_X86_64_KERNEL_ABI_SPINLOCK_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_KERNEL_ABI_SPINLOCK_HPP_

#include "spinlock.hpp"

#include <constants.hpp>
#include <extensions/debug.hpp>
#include <extensions/types.hpp>
#include <memory_io.hpp>

#include "core.hpp"

namespace arch
{
class alignas(kCacheLineSizeBytes) Spinlock : public SpinlockAbi
{
    FAST_CALL void Pause() { asm volatile("pause"); }

    struct DebugLock {
        u32 locked : 1;
        u32 owner : 16;
        u32 empty : 15;
    };

    static_assert(sizeof(DebugLock) == sizeof(u32));

    FORCE_INLINE_F void lock_debug_()
    {
        R_ASSERT_NEQ(
            GetCurrentCoreId() + 1, CastRegister<DebugLock>(lock_),
            "Double lock detected! Spinlock is already locked by core %d", GetCurrentCoreId()
        );

        const u32 value = ToRawRegister(DebugLock{
            .locked = 1,
            .owner  = GetCurrentCoreId() + 1,
            .empty  = 0,
        });

        u64 failed_lock_tries{};
        while (__builtin_expect(__sync_lock_test_and_set(&lock_, value), 0)) {
            Pause();
            ++failed_lock_tries;
        }

        TRACE_DEBUG(
            "Spinlock %p locked by core %d, failed tries: %d", this, GetCurrentCoreId(),
            failed_lock_tries
        );

        success_lock_tries_ += 1;
        failed_lock_tries_ += failed_lock_tries;
    }

    FORCE_INLINE_F void unlock_debug_()
    {
        R_ASSERT_TRUE(
            is_locked(), "Unlocking spinlock that is not locked!
        );

        R_ASSERT_EQ(
            GetCurrentCoreId() + 1, CastRegister<DebugLock>(lock_).owner,
            "Spinlock is locked by core %d, but unlocking by core %d",
            CastRegister<DebugLock>(lock_).owner - 1, GetCurrentCoreId()
        );

        __sync_lock_release(&lock_);
    }

    NODISCARD FORCE_INLINE_F bool try_lock_debug_()
    {
        R_ASSERT_NEQ(
            GetCurrentCoreId() + 1, CastRegister<DebugLock>(lock_),
            "Double lock detected! Spinlock is already locked by core %d", GetCurrentCoreId()
        );

        return !__sync_lock_test_and_set(&lock_, 1);
    }

    public:
    /* basic operations */
    FORCE_INLINE_F void lock()
    {
        if constexpr (kIsDebugBuild) {
            lock_debug_();
            return;
        }

        while (__builtin_expect(__sync_lock_test_and_set(&lock_, 1), 0)) {
            Pause();
        }
    }

    FORCE_INLINE_F void unlock()
    {
        if constexpr (kIsDebugBuild) {
            unlock_debug_();
            return;
        }

        __sync_lock_release(&lock_);
    }

    FORCE_INLINE_F NODISCARD bool try_lock()
    {
        if constexpr (kIsDebugBuild) {
            return try_lock_debug_();
        }

        return !__sync_lock_test_and_set(&lock_, 1);
    }

    FORCE_INLINE_F NODISCARD bool is_locked() const { return lock_ != 0; }

    protected:
    u32 lock_               = 0;
    u64 failed_lock_tries_  = 0;
    u64 success_lock_tries_ = 0;
};
}  // namespace arch

#endif  // ALKOS_KERNEL_ARCH_X86_64_KERNEL_ABI_SPINLOCK_HPP_
