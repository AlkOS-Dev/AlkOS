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
    EXPECT_EQ(3_s, std::tuple_size<decltype(tuple)>::value);

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

    EXPECT_EQ(4_s, std::tuple_size_v<decltype(tuple1)>);
}

TEST_F(TupleTest, TupleGet) {}

TEST_F(TupleTest, TupleElement) {}

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
    EXPECT_TRUE((std::is_same_v<const char* const, decltype(k)>));
}
