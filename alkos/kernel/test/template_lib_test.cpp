#include <extensions/template_lib.hpp>
#include <test_module/test.hpp>

using namespace TemplateLib;

class TemplateLibTest : public TestGroupBase
{
};

struct TestFunc {
    template <uint64_t N>
    void operator()(int &out) const
    {
        out = static_cast<int>(N);
    }
};

struct TestFuncReturn {
    template <uint64_t N>
    int operator()() const
    {
        return static_cast<int>(N);
    }
};

void DefaultFunc(int &out) { out = -1; }

int DefaultFuncReturn() { return -1; }

TEST_F(TemplateLibTest, RolledSwitchTest)
{
    int result                   = -1;
    constexpr uint64_t kMaxValue = 10;
    constexpr uint64_t kStep     = 2;

    // Test case where value matches max value
    RolledSwitch<int, kMaxValue, kStep>(DefaultFunc, TestFunc{}, 10, result);
    EXPECT_EQ(result, 10);

    // Test case where value is within range but not max
    RolledSwitch<int, kMaxValue, kStep>(DefaultFunc, TestFunc{}, 8, result);
    EXPECT_EQ(result, 8);

    // Test case where value does not match any case (fallback)
    RolledSwitch<int, kMaxValue, kStep>(DefaultFunc, TestFunc{}, 7, result);
    EXPECT_EQ(result, -1);

    // Test case where value is bigger than kMaxValue
    RolledSwitch<int, kMaxValue, kStep>(DefaultFunc, TestFunc{}, 11, result);
    EXPECT_EQ(result, -1);

    // Test case where value is zero
    RolledSwitch<int, kMaxValue, kStep>(DefaultFunc, TestFunc{}, 0, result);
    EXPECT_EQ(result, 0);
}

TEST_F(TemplateLibTest, RolledSwitchReturnable)
{
    constexpr uint64_t kMaxValue = 10;
    constexpr uint64_t kStep     = 2;

    // Test case for RolledSwitchReturnable with a matching value
    int returnResult =
        RolledSwitchReturnable<int, kMaxValue, kStep>(DefaultFuncReturn, TestFuncReturn{}, 10);
    EXPECT_EQ(returnResult, 10);

    // Test case for RolledSwitchReturnable with a non-matching value (fallback)
    returnResult =
        RolledSwitchReturnable<int, kMaxValue, kStep>(DefaultFuncReturn, TestFuncReturn{}, 7);
    EXPECT_EQ(returnResult, -1);

    // Test case for RolledSwitchReturnable with a value bigger than kMaxValue
    returnResult =
        RolledSwitchReturnable<int, kMaxValue, kStep>(DefaultFuncReturn, TestFuncReturn{}, 11);
    EXPECT_EQ(returnResult, -1);

    // Test case where value is zero
    returnResult =
        RolledSwitchReturnable<int, kMaxValue, kStep>(DefaultFuncReturn, TestFuncReturn{}, 0);
    EXPECT_EQ(returnResult, 0);
}

TEST_F(TemplateLibTest, CountTypeTest)
{
    EXPECT_EQ((CountType<int, int, float, double>()), 1_size);
    EXPECT_EQ((CountType<int, float, double>()), 0_size);
    EXPECT_EQ((CountType<int, int, int, int>()), 3_size);
    EXPECT_EQ((CountType<char, char, int, char, double, char>()), 3_size);
    EXPECT_EQ((CountType<float, int, double, float, float, float>()), 3_size);
}

TEST_F(TemplateLibTest, HasTypeTest)
{
    EXPECT_TRUE((HasType<int, int, float, double>()));
    EXPECT_FALSE((HasType<int, float, double>()));
}

TEST_F(TemplateLibTest, HasTypeOnceTest)
{
    EXPECT_TRUE((HasTypeOnce<int, int, float, double>()));
    EXPECT_FALSE((HasTypeOnce<int, int, int, double>()));
    EXPECT_FALSE((HasTypeOnce<int, float, double>()));
}

TEST_F(TemplateLibTest, HasDuplicateTypeTest)
{
    EXPECT_TRUE((HasDuplicateType<int, int, int, double>()));
    EXPECT_FALSE((HasDuplicateType<int, float, double>()));
}

TEST_F(TemplateLibTest, HasDuplicateTypesTest)
{
    EXPECT_TRUE((HasDuplicateTypes<int, int, float, double>()));
    EXPECT_TRUE((HasDuplicateTypes<int, int, double, double>()));
    EXPECT_TRUE((HasDuplicateTypes<int, float, double, float>()));
    EXPECT_FALSE((HasDuplicateTypes<int, float, double>()));
}

TEST_F(TemplateLibTest, TypeListTest)
{
    using TestList1 = TypeList<int, double, float>::Iterator<0>::type;
    EXPECT_TRUE((std::is_same_v<TestList1, int>));

    using TestList2 = TypeList<int, double, float>::Iterator<1>::type;
    EXPECT_TRUE((std::is_same_v<TestList2, double>));

    using TestList3 = TypeList<int, double, float>::Iterator<2>::type;
    EXPECT_TRUE((std::is_same_v<TestList3, float>));
}

TEST_F(TemplateLibTest, TypeListSizeTest)
{
    constexpr size_t size = TypeList<int, double, float>::kSize;
    EXPECT_EQ(size, 3_size);
}

template <size_t N>
using NewTypeIterator = typename TypeList<int, double, float>::Iterator<N>;

TEST_F(TemplateLibTest, TypeListOutOfBoundsTest)
{
    // This test should fail to compile if uncommented
    // using type = TypeList<int, double, float>::Iterator<3>::type;
}

struct NewTestFunctor {
    template <size_t Index, typename T>
    void operator()() const
    {
        if constexpr (Index == 0) {
            EXPECT_TRUE((std::is_same_v<T, int>));
        } else if constexpr (Index == 1) {
            EXPECT_TRUE((std::is_same_v<T, double>));
        } else if constexpr (Index == 2) {
            EXPECT_TRUE((std::is_same_v<T, float>));
        }
    }
};

TEST_F(TemplateLibTest, ApplyTest) { TypeList<int, double, float>::Apply(NewTestFunctor{}); }

TEST_F(TemplateLibTest, ApplyWithNTest)
{
    TypeList<int, double, float>::Apply<2>(NewTestFunctor{});
}

TEST_F(TemplateLibTest, GetTypeIndexTest)
{
    constexpr size_t index1 = GetTypeIndexInTypes<int, int, float, double>();
    EXPECT_EQ(index1, 0_size);

    constexpr size_t index2 = GetTypeIndexInTypes<float, int, float, double>();
    EXPECT_EQ(index2, 1_size);

    constexpr size_t index3 = GetTypeIndexInTypes<double, int, float, double>();
    EXPECT_EQ(index3, 2_size);
}

TEST_F(TemplateLibTest, GetTypeIndexFailsOnDuplicates)
{
    // This should fail to compile due to static_assert
    // constexpr size_t index = GetTypeIndexInTypes<int, int, int, float>();
}

TEST_F(TemplateLibTest, StaticSingletonTest)
{
    class TestSingleton : public StaticSingletonHelper
    {
        protected:
        TestSingleton(int a) { value = a; }

        public:
        int value;
    };

    StaticSingleton<TestSingleton>::Init(10);
    ASSERT_EQ(10, StaticSingleton<TestSingleton>::Get().value);
}

TEST_F(TemplateLibTest, StaticSingletonAlignTest)
{
    class TestSingleton : public StaticSingletonHelper
    {
        protected:
        TestSingleton() = default;

        public:
        alignas(512) char tab[1024];
    };

    const TestSingleton *addr = &StaticSingleton<TestSingleton>::Init();
    const uint64_t align      = reinterpret_cast<uint64_t>(addr) % 512;
    ASSERT_EQ(0LLU, align);
}
