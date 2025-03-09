#include <extensions/tuple.hpp>
#include <extensions/type_traits.hpp>
#include <test_module/test.hpp>

class TupleTest : public TestGroupBase
{
};

TEST_F(TupleTest, TupleBasics)
{
    const auto tuple = std::make_tuple(1, 2, 3);
    EXPECT_EQ(1, std::get<0>(tuple));
    EXPECT_EQ(2, std::get<1>(tuple));
    EXPECT_EQ(3, std::get<2>(tuple));
    EXPECT_EQ(3_size, std::tuple_size<decltype(tuple)>::value);

    EXPECT_TRUE((std::is_same_v<const int, std::tuple_element_t<0, decltype(tuple)>>));
    EXPECT_TRUE((std::is_same_v<const int, std::tuple_element_t<1, decltype(tuple)>>));
    EXPECT_TRUE((std::is_same_v<const int, std::tuple_element_t<2, decltype(tuple)>>));

    auto tuple1 = std::make_tuple(1, 2, 3);
    EXPECT_EQ(1, std::get<0>(tuple1));
    EXPECT_EQ(2, std::get<1>(tuple1));
    EXPECT_EQ(3, std::get<2>(tuple1));

    EXPECT_TRUE((std::is_same_v<int, std::tuple_element_t<0, decltype(tuple1)>>));
    EXPECT_TRUE((std::is_same_v<int, std::tuple_element_t<1, decltype(tuple1)>>));
    EXPECT_TRUE((std::is_same_v<int, std::tuple_element_t<2, decltype(tuple1)>>));
}

TEST_F(TupleTest, TupleMixedTypes)
{
    const char* str   = "abcd";
    const auto tuple1 = std::make_tuple(1, 1.0, 2.0f, str);
    EXPECT_EQ(static_cast<int>(1), tuple1.get<0>());
    EXPECT_EQ(1.0, tuple1.get<1>());
    EXPECT_EQ(2.0f, tuple1.get<2>());
    EXPECT_STREQ(str, tuple1.get<3>());

    EXPECT_TRUE((std::is_same_v<const int, std::tuple_element_t<0, decltype(tuple1)>>));
    EXPECT_TRUE((std::is_same_v<const double, std::tuple_element_t<1, decltype(tuple1)>>));
    EXPECT_TRUE((std::is_same_v<const float, std::tuple_element_t<2, decltype(tuple1)>>));
    EXPECT_TRUE((std::is_same_v<const char* const, std::tuple_element_t<3, decltype(tuple1)>>));

    EXPECT_EQ(4_size, std::tuple_size_v<decltype(tuple1)>);
}

TEST_F(TupleTest, TupleGet)
{
    auto tuple = std::make_tuple(42, "hello", 3.14);

    EXPECT_EQ(42, std::get<0>(tuple));
    EXPECT_STREQ("hello", std::get<1>(tuple));
    EXPECT_EQ(3.14, std::get<2>(tuple));

    std::get<0>(tuple) = 100;
    EXPECT_EQ(100, std::get<0>(tuple));

    const auto constTuple = std::make_tuple(1, 2.5, "test");
    EXPECT_EQ(1, std::get<0>(constTuple));
    EXPECT_EQ(2.5, std::get<1>(constTuple));
    EXPECT_STREQ("test", std::get<2>(constTuple));

    EXPECT_EQ(100, std::get<int>(tuple));
    EXPECT_STREQ("hello", std::get<const char*>(tuple));
    EXPECT_EQ(3.14, std::get<double>(tuple));

    EXPECT_EQ(10, std::get<0>(std::make_tuple(10, 20, 30)));
}

TEST_F(TupleTest, TupleElement)
{
    using TupleType = std::tuple<int, double, const char*>;

    EXPECT_TRUE((std::is_same_v<int, std::tuple_element_t<0, TupleType>>));
    EXPECT_TRUE((std::is_same_v<double, std::tuple_element_t<1, TupleType>>));
    EXPECT_TRUE((std::is_same_v<const char*, std::tuple_element_t<2, TupleType>>));

    using ConstTupleType = const std::tuple<int, double, const char*>;
    EXPECT_TRUE((std::is_same_v<const int, std::tuple_element_t<0, ConstTupleType>>));
    EXPECT_TRUE((std::is_same_v<const double, std::tuple_element_t<1, ConstTupleType>>));
    EXPECT_TRUE((std::is_same_v<const char* const, std::tuple_element_t<2, ConstTupleType>>));

    using VolatileTupleType = volatile std::tuple<int, double, const char*>;
    EXPECT_TRUE((std::is_same_v<volatile int, std::tuple_element_t<0, VolatileTupleType>>));

    using CVTupleType = const volatile std::tuple<int, double, const char*>;
    EXPECT_TRUE((std::is_same_v<const volatile int, std::tuple_element_t<0, CVTupleType>>));
}

TEST_F(TupleTest, TupleBindings)
{
    const auto tuple     = std::make_tuple(1, 2, 3);
    const auto [a, b, c] = tuple;
    EXPECT_EQ(1, a);
    EXPECT_EQ(2, b);
    EXPECT_EQ(3, c);

    EXPECT_TRUE((std::is_same_v<const int, decltype(a)>));
    EXPECT_TRUE((std::is_same_v<const int, decltype(b)>));
    EXPECT_TRUE((std::is_same_v<const int, decltype(c)>));

    static constexpr const char* str = "abcd";
    const auto tuple1                = std::make_tuple(static_cast<int>(1), 1.0, 2.0f, str);

    const auto [d, e, f, g] = tuple1;
    EXPECT_EQ(static_cast<int>(1), d);
    EXPECT_EQ(1.0, e);
    EXPECT_EQ(2.0f, f);
    EXPECT_STREQ(str, g);

    EXPECT_TRUE((std::is_same_v<const int, decltype(d)>));
    EXPECT_TRUE((std::is_same_v<const double, decltype(e)>));
    EXPECT_TRUE((std::is_same_v<const float, decltype(f)>));
    EXPECT_TRUE((std::is_same_v<const char* const, decltype(g)>));

    auto tuple2       = std::make_tuple(static_cast<int>(1), 1.0, 2.0f, str);
    auto [h, i, j, k] = tuple2;
    EXPECT_EQ(static_cast<int>(1), h);
    EXPECT_EQ(1.0, i);
    EXPECT_EQ(2.0f, j);
    EXPECT_STREQ(str, k);

    EXPECT_TRUE((std::is_same_v<int, decltype(h)>));
    EXPECT_TRUE((std::is_same_v<double, decltype(i)>));
    EXPECT_TRUE((std::is_same_v<float, decltype(j)>));
    EXPECT_TRUE((std::is_same_v<const char*, decltype(k)>));
}

TEST_F(TupleTest, TupleHelpers)
{
    auto t1 = std::make_tuple(42, "test", 3.14);
    EXPECT_EQ(42, std::get<0>(t1));
    EXPECT_STREQ("test", std::get<1>(t1));
    EXPECT_EQ(3.14, std::get<2>(t1));

    int arr[3] = {1, 2, 3};
    auto t2    = std::make_tuple(arr, 5);
    EXPECT_TRUE((std::is_same_v<int*, std::tuple_element_t<0, decltype(t2)>>));

    TODO_FULL_TUPLE_SUPPORT
    // // Test make_tuple with references using std::ref and std::cref
    // int x = 42;
    // auto t3 = std::make_tuple(std::ref(x), std::cref(x));
    // x = 100;
    // EXPECT_EQ(100, std::get<0>(t3));
    // EXPECT_EQ(100, std::get<1>(t3));

    // Test type correctness with ref
    // EXPECT_TRUE((std::is_same_v<std::reference_wrapper<int>, std::tuple_element_t<0,
    // decltype(t3)>>)); EXPECT_TRUE((std::is_same_v<std::reference_wrapper<const int>,
    // std::tuple_element_t<1, decltype(t3)>>));
}

TODO_FULL_TUPLE_SUPPORT
// TEST_F(TupleTest, TupleComparison)
// {
//     auto tuple1 = std::make_tuple(1, 2.0, "a");
//     auto tuple2 = std::make_tuple(1, 2.0, "a");
//     auto tuple3 = std::make_tuple(2, 2.0, "a");
//     auto tuple4 = std::make_tuple(1, 3.0, "a");
//     auto tuple5 = std::make_tuple(1, 2.0, "b");
//
//     EXPECT_TRUE(tuple1 == tuple2);
//     EXPECT_FALSE(tuple1 == tuple3);
//     EXPECT_FALSE(tuple1 == tuple4);
//     EXPECT_FALSE(tuple1 == tuple5);
//
//     EXPECT_FALSE(tuple1 != tuple2);
//     EXPECT_TRUE(tuple1 != tuple3);
//     EXPECT_TRUE(tuple1 != tuple4);
//     EXPECT_TRUE(tuple1 != tuple5);
//
//     EXPECT_TRUE(tuple1 < tuple3);
//     EXPECT_TRUE(tuple1 < tuple4);
//     EXPECT_TRUE(tuple1 < tuple5);
//     EXPECT_FALSE(tuple3 < tuple1);
//
//     EXPECT_TRUE(tuple1 <= tuple2);
//     EXPECT_TRUE(tuple1 <= tuple3);
//     EXPECT_TRUE(tuple1 <= tuple5);
//     EXPECT_FALSE(tuple3 <= tuple1);
//
//     EXPECT_FALSE(tuple1 > tuple2);
//     EXPECT_FALSE(tuple1 > tuple3);
//     EXPECT_TRUE(tuple3 > tuple1);
//     EXPECT_TRUE(tuple5 > tuple1);
//
//     EXPECT_TRUE(tuple1 >= tuple2);
//     EXPECT_FALSE(tuple1 >= tuple3);
//     EXPECT_TRUE(tuple3 >= tuple1);
// }

TEST_F(TupleTest, TupleCopyMove)
{
    auto original = std::make_tuple(42, 3.14, "test");
    auto copy     = original;

    EXPECT_EQ(std::get<0>(original), std::get<0>(copy));
    EXPECT_EQ(std::get<1>(original), std::get<1>(copy));
    EXPECT_STREQ(std::get<2>(original), std::get<2>(copy));

    std::get<0>(copy) = 100;

    EXPECT_EQ(42, std::get<0>(original));
    EXPECT_STREQ("test", std::get<2>(original));

    auto movable = std::make_tuple(42, 3.14, "test");
    auto moved   = std::move(movable);

    EXPECT_EQ(42, std::get<0>(moved));
    EXPECT_EQ(3.14, std::get<1>(moved));
    EXPECT_STREQ("test", std::get<2>(moved));

    auto assigned = std::tuple(moved);

    EXPECT_EQ(42, std::get<0>(assigned));
    EXPECT_EQ(3.14, std::get<1>(assigned));
    EXPECT_STREQ("test", std::get<2>(assigned));

    TODO_FULL_TUPLE_SUPPORT
    // auto target = std::make_tuple(0, 0.0, "");
    // target = std::make_tuple(100, 2.71, "moved");
    //
    // EXPECT_EQ(100, std::get<0>(target));
    // EXPECT_EQ(2.71, std::get<1>(target));
    // EXPECT_STREQ("moved", std::get<2>(target));
}

TEST_F(TupleTest, TupleEmptyConstruction)
{
    std::tuple<> emptyTuple{};
    EXPECT_EQ(0_size, std::tuple_size_v<decltype(emptyTuple)>);

    std::tuple<int, double, const char*> defaultTuple{};
    EXPECT_EQ(0, std::get<0>(defaultTuple));
    EXPECT_EQ(0.0, std::get<1>(defaultTuple));
    EXPECT_EQ(nullptr, std::get<2>(defaultTuple));
}
