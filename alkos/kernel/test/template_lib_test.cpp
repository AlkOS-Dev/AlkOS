#include <extensions/template_lib.hpp>
#include <test_module/test.hpp>

using namespace TemplateLib;

class TemplateLibTest : public TestGroupBase
{
};

struct TestFunc {
    template <uint64_t N>
    void operator()(int& out) const
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

void DefaultFunc(int& out) { out = -1; }

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

    const TestSingleton* addr = &StaticSingleton<TestSingleton>::Init();
    const uint64_t align      = reinterpret_cast<uint64_t>(addr) % 512;
    ASSERT_EQ(0LLU, align);
}

using EventCallbackFunc = void (*)(int);
struct CallbackCounters {
    int count1{};
    int count2{};
    int count3{};
    int totalSum{};
};

static void IncrementCounter1(CallbackCounters* counters, int value)
{
    counters->count1++;
    counters->totalSum += value;
}

static void IncrementCounter2(CallbackCounters* counters, int value)
{
    counters->count2++;
    counters->totalSum += value * 2;
}

static void IncrementCounter3(CallbackCounters* counters, int value)
{
    counters->count3++;
    counters->totalSum += value * 3;
}

TEST_F(TemplateLibTest, StaticEventTableTest)
{
    constexpr size_t kTableSize = 3;

    struct EventCallback {
        CallbackCounters* counters;
        void (*func)(CallbackCounters*, int);

        void operator()(int value) const { func(counters, value); }
    };

    StaticEventTable<kTableSize, EventCallback> eventTable;
    CallbackCounters counters{};

    eventTable.RegisterEvent<0>(EventCallback{&counters, IncrementCounter1});
    eventTable.RegisterEvent<0>(EventCallback{&counters, IncrementCounter2});
    eventTable.RegisterEvent<1>(EventCallback{&counters, IncrementCounter3});

    eventTable.Notify<0>(10);
    EXPECT_EQ(counters.count1, 1);
    EXPECT_EQ(counters.count2, 1);
    EXPECT_EQ(counters.count3, 0);
    EXPECT_EQ(counters.totalSum, 10 + 10 * 2);  // 30

    eventTable.Notify<1>(10);
    EXPECT_EQ(counters.count1, 1);
    EXPECT_EQ(counters.count2, 1);
    EXPECT_EQ(counters.count3, 1);
    EXPECT_EQ(counters.totalSum, 30 + 10 * 3);  // 60

    eventTable.Notify<0>(5);
    EXPECT_EQ(counters.count1, 2);
    EXPECT_EQ(counters.count2, 2);
    EXPECT_EQ(counters.count3, 1);
    EXPECT_EQ(counters.totalSum, 60 + 5 + 5 * 2);  // 75
}

struct TestData {
    int value;
    bool operator==(const TestData& other) const { return value == other.value; }
};

TEST_F(TemplateLibTest, SettingsTest)
{
    using TestTypeList = TypeList<int, float, TestData>;

    Settings<TestTypeList> settings({42, 3.14f, TestData{100}});

    EXPECT_EQ(settings.Get<0>(), 42);
    EXPECT_EQ(settings.Get<1>(), 3.14f);
    EXPECT_EQ(settings.Get<2>().value, 100);

    settings.Set<0>(100);
    settings.Set<1>(2.71f);
    settings.Set<2>(TestData{200});

    EXPECT_EQ(settings.Get<0>(), 100);
    EXPECT_EQ(settings.Get<1>(), 2.71f);
    EXPECT_EQ(settings.Get<2>().value, 200);
}

using TestTypeList = TypeList<int, float, TestData>;
struct EventCounters {
    int intChanged   = 0;
    float floatValue = 0.0f;
    int dataChanged  = 0;
};

// Global data for callback functions
EventCounters g_counters;
Settings<TypeList<int, float, TestData>>* g_settings_ptr = nullptr;

// Callback functions
void IntChangedCallback() { g_counters.intChanged++; }

void FloatValueCallback() { g_counters.floatValue = g_settings_ptr->Get<1>(); }

void DataChangedCallback() { g_counters.dataChanged++; }

TEST_F(TemplateLibTest, SettingsWithEventsTest)
{
    g_counters = EventCounters{};

    Settings<TestTypeList> settings({42, 3.14f, TestData{100}});
    g_settings_ptr = &settings;

    settings.RegisterEvent<0>(IntChangedCallback);
    settings.RegisterEvent<1>(FloatValueCallback);
    settings.RegisterEvent<2>(DataChangedCallback);

    settings.SetAndNotify<0>(100);
    EXPECT_EQ(g_counters.intChanged, 1);
    EXPECT_EQ(settings.Get<0>(), 100);

    settings.SetAndNotify<1>(2.71f);
    EXPECT_EQ(g_counters.floatValue, 2.71f);
    EXPECT_EQ(settings.Get<1>(), 2.71f);

    settings.SetAndNotify<2>(TestData{200});
    EXPECT_EQ(g_counters.dataChanged, 1);
    EXPECT_EQ(settings.Get<2>().value, 200);

    settings.SetAndNotify<0>(200);
    settings.SetAndNotify<0>(300);
    EXPECT_EQ(g_counters.intChanged, 3);
    EXPECT_EQ(settings.Get<0>(), 300);
}

int g_changeCounter = 0;

void ChangeCounterCallback() { g_changeCounter++; }

TEST_F(TemplateLibTest, EmptySettingsTest)
{
    /* Compilation check */
    using EmptyTypeList = TypeList<>;
    [[maybe_unused]] Settings<EmptyTypeList> emptySettings({});
    EXPECT_TRUE(true);
}

TEST_F(TemplateLibTest, SingleTypeSettingsTest)
{
    using SingleTypeList = TypeList<int>;
    Settings<SingleTypeList> singleSettings(std::tuple{42});

    EXPECT_EQ(singleSettings.Get<0>(), 42);

    singleSettings.Set<0>(100);
    EXPECT_EQ(singleSettings.Get<0>(), 100);

    g_changeCounter = 0;
    singleSettings.RegisterEvent<0>(ChangeCounterCallback);

    singleSettings.SetAndNotify<0>(200);
    EXPECT_EQ(g_changeCounter, 1);
    EXPECT_EQ(singleSettings.Get<0>(), 200);
}

int g_counter1 = 0;
int g_counter2 = 0;
int g_counter3 = 0;

void Counter1Callback() { g_counter1++; }
void Counter2Callback() { g_counter2++; }
void Counter3Callback() { g_counter3++; }

TEST_F(TemplateLibTest, MultipleEventSubscribersTest)
{
    using TestTypeList = TypeList<int>;
    Settings<TestTypeList> settings(std::tuple<int>{42});

    g_counter1 = 0;
    g_counter2 = 0;
    g_counter3 = 0;

    settings.RegisterEvent<0>(Counter1Callback);
    settings.RegisterEvent<0>(Counter2Callback);
    settings.RegisterEvent<0>(Counter3Callback);

    settings.SetAndNotify<0>(100);
    EXPECT_EQ(g_counter1, 1);
    EXPECT_EQ(g_counter2, 1);
    EXPECT_EQ(g_counter3, 1);

    settings.SetAndNotify<0>(200);
    EXPECT_EQ(g_counter1, 2);
    EXPECT_EQ(g_counter2, 2);
    EXPECT_EQ(g_counter3, 2);
}
