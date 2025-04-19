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
    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    ~Spinlock()
    {
        if constexpr (kIsDebugBuild) {
            R_ASSERT_FALSE(
                IsLocked(), "Spinlock is locked, but destructor is called (%p, core: %hu)", this,
                CastRegister<DebugLock>(lock_).owner - 1
            );
        }
    }

    // ------------------------------
    // ABI implementations
    // ------------------------------

    FORCE_INLINE_F void Lock()
    {
        if constexpr (kIsDebugBuild) {
            LockDebug_();
            return;
        }

        while (__builtin_expect(__sync_lock_test_and_set(&lock_, 1), 0)) {
            Pause_();
        }
    }

    FORCE_INLINE_F void Unlock()
    {
        if constexpr (kIsDebugBuild) {
            UnlockDebug_();
            return;
        }

        __sync_lock_release(&lock_);
    }

    FORCE_INLINE_F NODISCARD bool TryLock()
    {
        if constexpr (kIsDebugBuild) {
            return TryLockDebug_();
        }

        return !__sync_lock_test_and_set(&lock_, 1);
    }

    FORCE_INLINE_F NODISCARD bool IsLocked() const { return lock_ != 0; }

    // ------------------------------
    // Class defines
    // ------------------------------

    struct DebugLock {
        u32 locked : 1;
        u32 owner : 16;
        u32 empty : 15;
    };

    static_assert(sizeof(DebugLock) == sizeof(u32));

    // ------------------------------
    // Private methods
    // ------------------------------

    FAST_CALL void Pause_() { asm volatile("pause"); }

    FORCE_INLINE_F void LockDebug_()
    {
        R_ASSERT_NEQ(
            GetCurrentCoreId() + 1, CastRegister<DebugLock>(lock_).owner,
            "Double lock detected! Spinlock is already locked by core %d", GetCurrentCoreId()
        );

        const u32 value = ToRawRegister(DebugLock{
            .locked = 1,
            .owner  = GetCurrentCoreId() + 1,
            .empty  = 0,
        });

        u64 failed_lock_tries{};
        while (__builtin_expect(__sync_val_compare_and_swap(&lock_, 0, value) != 0, 0)) {
            Pause_();
            ++failed_lock_tries;
        }

        TRACE_DEBUG(
            "Spinlock %p locked by core %d, failed tries: %d", this, GetCurrentCoreId(),
            failed_lock_tries
        );

        success_lock_tries_ += 1;
        failed_lock_tries_ += failed_lock_tries;
    }

    FORCE_INLINE_F void UnlockDebug_()
    {
        R_ASSERT_TRUE(IsLocked(), "Unlocking spinlock that is not locked!");

        R_ASSERT_EQ(
            GetCurrentCoreId() + 1, CastRegister<DebugLock>(lock_).owner,
            "Spinlock is locked by core %d, but unlocking by core %d",
            CastRegister<DebugLock>(lock_).owner - 1, GetCurrentCoreId()
        );

        __sync_lock_release(&lock_);
    }

    NODISCARD FORCE_INLINE_F bool TryLockDebug_()
    {
        R_ASSERT_NEQ(
            GetCurrentCoreId() + 1, CastRegister<DebugLock>(lock_).owner,
            "Double lock detected! Spinlock is already locked by core %d", GetCurrentCoreId()
        );

        const u32 value = ToRawRegister(DebugLock{
            .locked = 1,
            .owner  = GetCurrentCoreId() + 1,
            .empty  = 0,
        });

        return __sync_bool_compare_and_swap(&lock_, 0, value);
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    protected:
    volatile u32 lock_                       = 0;
    [[maybe_unused]] u64 failed_lock_tries_  = 0;
    [[maybe_unused]] u64 success_lock_tries_ = 0;
};
}  // namespace arch

#endif  // ALKOS_KERNEL_ARCH_X86_64_KERNEL_ABI_SPINLOCK_HPP_
