#include <test_module/test.hpp>

#include <extensions/defines.hpp>
#include <extensions/optional.hpp>

class OptionalTestClass : public TestGroupBase
{
};

class TestClass
{
    public:
    explicit TestClass(const int x) : m_a(x), m_b(2 * x + 2) {}

    int m_a;
    const int m_b;
};

TEST_F(OptionalTestClass, OptionalEmplace)
{
    std::optional<TestClass> opt;

    TestClass& ref = opt.emplace(42);
    R_ASSERT_TRUE(opt.has_value());
    R_ASSERT_EQ(42, ref.m_a);
    R_ASSERT_EQ(86, ref.m_b);

    opt.emplace(10);
    R_ASSERT_TRUE(opt.has_value());
    R_ASSERT_EQ(10, opt->m_a);
    R_ASSERT_EQ(22, opt->m_b);
}

TEST_F(OptionalTestClass, OptionalReset)
{
    std::optional<TestClass> opt;

    opt.reset();
    R_ASSERT_FALSE(opt.has_value());

    opt.emplace(5);
    R_ASSERT_TRUE(opt.has_value());

    opt.reset();
    R_ASSERT_FALSE(opt.has_value());

    opt.reset();
    R_ASSERT_FALSE(opt.has_value());
}

TEST_F(OptionalTestClass, OptionalDereference)
{
    std::optional<TestClass> opt;
    opt.emplace(7);

    TestClass& ref = *opt;
    R_ASSERT_EQ(7, ref.m_a);
    R_ASSERT_EQ(16, ref.m_b);

    const auto& const_opt      = opt;
    const TestClass& const_ref = *const_opt;
    R_ASSERT_EQ(7, const_ref.m_a);
    R_ASSERT_EQ(16, const_ref.m_b);

    ref.m_a = 100;
    R_ASSERT_EQ(100, opt->m_a);
}

TEST_F(OptionalTestClass, OptionalArrow)
{
    std::optional<TestClass> opt;
    opt.emplace(15);

    R_ASSERT_EQ(15, opt->m_a);
    R_ASSERT_EQ(32, opt->m_b);

    const auto& const_opt = opt;
    R_ASSERT_EQ(15, const_opt->m_a);
    R_ASSERT_EQ(32, const_opt->m_b);

    opt->m_a = 200;
    R_ASSERT_EQ(200, opt->m_a);
}

TEST_F(OptionalTestClass, OptionalBoolConversion)
{
    std::optional<int> opt;

    R_ASSERT_FALSE(static_cast<bool>(opt));
    R_ASSERT_FALSE(opt);

    opt.emplace(0);  // Even with value 0
    R_ASSERT_TRUE(static_cast<bool>(opt));
    R_ASSERT_TRUE(opt);

    opt.reset();
    R_ASSERT_FALSE(static_cast<bool>(opt));
}

TEST_F(OptionalTestClass, OptionalAlignment)
{
    struct AlignedStruct {
        alignas(32) char data[32];
        int value;

        explicit AlignedStruct(int v) : value(v)
        {
            for (int i = 0; i < 32; ++i) data[i] = static_cast<char>(i);
        }
    };

    std::optional<AlignedStruct> opt;
    opt.emplace(123);

    R_ASSERT_TRUE(opt.has_value());
    R_ASSERT_EQ(123, opt->value);

    // Check that the memory is properly aligned
    // Note: This is implementation-dependent, but should work with aligned_storage
    uintptr_t addr = reinterpret_cast<uintptr_t>(&(*opt));
    R_ASSERT_EQ(0_size, addr % 32);
}

TEST_F(OptionalTestClass, OptionalZeroInit)
{
    std::optional<int> opt;
    opt.emplace();

    R_ASSERT_TRUE(opt.has_value());
    R_ASSERT_EQ(0, *opt);
}
