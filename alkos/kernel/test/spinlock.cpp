/* internal includes */

#include <assert.h>
#include <extensions/mutex.hpp>
#include <sync/kernel/spinlock.hpp>
#include <test_module/test.hpp>

class SpinlockTest : public TestGroupBase
{
    protected:
    Spinlock lock_{};
};

// ------------------------------
// tests
// ------------------------------

TEST_F(SpinlockTest, SimpleLock)
{
    lock_.Lock();
    R_ASSERT_TRUE(lock_.IsLocked());
    lock_.Unlock();
    R_ASSERT_FALSE(lock_.IsLocked());
}

TEST_F(SpinlockTest, TryLock)
{
    R_ASSERT_TRUE(lock_.TryLock());
    R_ASSERT_TRUE(lock_.IsLocked());
    lock_.Unlock();
    R_ASSERT_FALSE(lock_.IsLocked());
}

TEST_F(SpinlockTest, LockGuard)
{
    {
        std::lock_guard lock(lock_);
        R_ASSERT_TRUE(lock_.IsLocked());
    }
    R_ASSERT_FALSE(lock_.IsLocked());
}

/**
 * @note TESTS BELOW WILL ONLY WORK CORRECTLY IN DEBUG BUILD
 */

FAIL_TEST_F(SpinlockTest, DoubleLock)
{
    lock_.Lock();
    R_ASSERT_TRUE(lock_.IsLocked());
    lock_.Lock();
}

FAIL_TEST_F(SpinlockTest, DoubleLockWithTryLock)
{
    lock_.Lock();
    R_ASSERT_TRUE(lock_.IsLocked());
    R_ASSERT_FALSE(lock_.TryLock());
    lock_.Unlock();
}

FAIL_TEST_F(SpinlockTest, UnlockWithoutLock) { lock_.Unlock(); }

FAIL_TEST_F(SpinlockTest, LockedLeftoverLock) { lock_.Lock(); }
