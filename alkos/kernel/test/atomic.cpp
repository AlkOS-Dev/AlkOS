#include <extensions/atomic.hpp>
#include <test_module/test.hpp>

class AtomicTest : public TestGroupBase
{
};

TEST_F(AtomicTest, AtomicInt)
{
    std::atomic<int> a{42};
    ASSERT_EQ(a.load(), 42);
    a.store(100);
    ASSERT_EQ(a.load(), 100);
    ASSERT_EQ(a.fetch_add(1), 100);
    ASSERT_EQ(a.load(), 101);
    ASSERT_EQ(a.fetch_sub(1), 101);
    ASSERT_EQ(a.load(), 100);
}
