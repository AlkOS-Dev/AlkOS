#include <mutex.hpp>
#include <sync/kernel/spinlock.hpp>
#include <test_module/test.hpp>

class MutexTest : public TestGroupBase
{
    protected:
    Spinlock lock_{};
};

TEST_F(MutexTest, LockGuard)
{
    {
        std::lock_guard lock(lock_);
        R_ASSERT_TRUE(lock_.IsLocked());
    }
    R_ASSERT_FALSE(lock_.IsLocked());
}
