#include <test_module/test.hpp>

#include <defines.hpp>
#include <optional.hpp>

class TestClass
{
    public:
    explicit TestClass(const int x) : m_a(x), m_b(2 * x + 2) {}

    int m_a;
    const int m_b;
};

TestClass g_GlobalTestClass(5);

/**
 * @brief no exception should be thrown and global object should be correctly initialized
 */
TEST(CppTest)
{
    char mem[sizeof(TestClass)];

    /* Test static initializer */
    static TestClass test(5);
    test.m_a++;

    /* test usual new and delete */
    const auto *test2 = new int;
    delete test2;

    /* test array new and delete */
    const auto *test3 = new int[5];
    delete[] test3;

    /* test placement new and delete */
    auto *test4 = new (reinterpret_cast<TestClass *>(mem)) TestClass(5);
    test4->m_a++;

    /* test global object */
    R_ASSERT_EQ(12, g_GlobalTestClass.m_b);
}

class AlignedTestClass : public TestGroupBase
{
    public:
    alignas(256) char m_mem[256];
};

TEST_F(AlignedTestClass, TestAlignas)
{
    R_ASSERT_EQ(0_size, reinterpret_cast<size_t>(m_mem) % 256_size);
}
