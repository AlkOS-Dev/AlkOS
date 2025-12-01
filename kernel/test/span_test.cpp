#include <array.hpp>
#include <span.hpp>
#include <test_module/test.hpp>

class SpanTest : public TestGroupBase
{
};

// ------------------------------
// Basic Construction Tests
// ------------------------------

TEST_F(SpanTest, DefaultConstructor)
{
    std::span<int> span;
    EXPECT_EQ(0_size, span.size());
    EXPECT_EQ(true, span.empty());
    EXPECT_EQ(nullptr, span.data());
}

TEST_F(SpanTest, FixedExtentEmptySpan)
{
    std::span<int, 0> span;
    EXPECT_EQ(0_size, span.size());
    EXPECT_EQ(true, span.empty());
    EXPECT_EQ(0_size, span.extent);
}

TEST_F(SpanTest, FromIterator)
{
    int arr[] = {1, 2, 3, 4, 5};
    std::span<int> span(arr, 5);

    EXPECT_EQ(5_size, span.size());
    EXPECT_EQ(false, span.empty());
    EXPECT_EQ(1, span[0]);
    EXPECT_EQ(2, span[1]);
    EXPECT_EQ(3, span[2]);
    EXPECT_EQ(4, span[3]);
    EXPECT_EQ(5, span[4]);
}

TEST_F(SpanTest, FromIteratorRange)
{
    int arr[] = {10, 20, 30, 40};
    std::span<int> span(arr, arr + 4);

    EXPECT_EQ(4_size, span.size());
    EXPECT_EQ(false, span.empty());
    EXPECT_EQ(10, span[0]);
    EXPECT_EQ(20, span[1]);
    EXPECT_EQ(30, span[2]);
    EXPECT_EQ(40, span[3]);
}

TEST_F(SpanTest, FromCStyleArray)
{
    int arr[] = {1, 2, 3, 4};
    std::span<int> span(arr);

    EXPECT_EQ(4_size, span.size());
    EXPECT_EQ(false, span.empty());
    EXPECT_EQ(1, span[0]);
    EXPECT_EQ(2, span[1]);
    EXPECT_EQ(3, span[2]);
    EXPECT_EQ(4, span[3]);
}

TEST_F(SpanTest, FromFixedCStyleArray)
{
    int arr[] = {10, 20, 30};
    std::span<int, 3> span(arr);

    EXPECT_EQ(3_size, span.size());
    EXPECT_EQ(3_size, span.extent);
    EXPECT_EQ(10, span[0]);
    EXPECT_EQ(20, span[1]);
    EXPECT_EQ(30, span[2]);
}

TEST_F(SpanTest, FromStdArray)
{
    std::array<int, 4> arr = {5, 6, 7, 8};
    std::span<int> span(arr);

    EXPECT_EQ(4_size, span.size());
    EXPECT_EQ(5, span[0]);
    EXPECT_EQ(6, span[1]);
    EXPECT_EQ(7, span[2]);
    EXPECT_EQ(8, span[3]);
}

TEST_F(SpanTest, FromInitializerList)
{
    std::initializer_list<int> il = {1, 2, 3, 4, 5};
    std::span<const int> span(il);

    EXPECT_EQ(5_size, span.size());
    EXPECT_EQ(1, span[0]);
    EXPECT_EQ(2, span[1]);
    EXPECT_EQ(3, span[2]);
    EXPECT_EQ(4, span[3]);
    EXPECT_EQ(5, span[4]);
}

TEST_F(SpanTest, FromFixedSpan)
{
    int arr[] = {1, 2, 3, 4};
    std::span<int, 4> fixed_span(arr);
    std::span<int> dynamic_span(fixed_span);

    EXPECT_EQ(4_size, dynamic_span.size());
    EXPECT_EQ(1, dynamic_span[0]);
    EXPECT_EQ(2, dynamic_span[1]);
    EXPECT_EQ(3, dynamic_span[2]);
    EXPECT_EQ(4, dynamic_span[3]);
}

TEST_F(SpanTest, FromDynamicSpan)
{
    int arr[] = {10, 20, 30};
    std::span<int> dynamic_span(arr);
    std::span<const int, 3> fixed_span(dynamic_span);

    EXPECT_EQ(3_size, fixed_span.size());
    EXPECT_EQ(10, fixed_span[0]);
    EXPECT_EQ(20, fixed_span[1]);
    EXPECT_EQ(30, fixed_span[2]);
}

// ------------------------------
// Element Access Tests
// ------------------------------

TEST_F(SpanTest, ElementAccess)
{
    int arr[] = {10, 20, 30, 40};
    std::span<int> span(arr);

    EXPECT_EQ(10, span[0]);
    EXPECT_EQ(20, span[1]);
    EXPECT_EQ(30, span[2]);
    EXPECT_EQ(40, span[3]);

    EXPECT_EQ(10, span.front());
    EXPECT_EQ(40, span.back());

    EXPECT_EQ(10, *(span.data()));
    EXPECT_EQ(20, *(span.data() + 1));
}

TEST_F(SpanTest, ModifyThroughSpan)
{
    int arr[] = {1, 2, 3};
    std::span<int> span(arr);

    span[0] = 10;
    span[1] = 20;
    span[2] = 30;

    EXPECT_EQ(10, arr[0]);
    EXPECT_EQ(20, arr[1]);
    EXPECT_EQ(30, arr[2]);
}

// ------------------------------
// Iterator Tests
// ------------------------------

TEST_F(SpanTest, IteratorAccess)
{
    int arr[] = {10, 20, 30};
    std::span<int> span(arr);

    auto it = span.begin();
    EXPECT_EQ(10, *it);
    ++it;
    EXPECT_EQ(20, *it);
    ++it;
    EXPECT_EQ(30, *it);
    ++it;
    EXPECT_EQ(span.end(), it);
}

// ------------------------------
// Subspan Tests
// ------------------------------

TEST_F(SpanTest, FirstSubspan)
{
    int arr[] = {1, 2, 3, 4, 5};
    std::span<int> span(arr);

    auto sub = span.first(3);
    EXPECT_EQ(3_size, sub.size());
    EXPECT_EQ(1, sub[0]);
    EXPECT_EQ(2, sub[1]);
    EXPECT_EQ(3, sub[2]);
}

TEST_F(SpanTest, LastSubspan)
{
    int arr[] = {1, 2, 3, 4, 5};
    std::span<int> span(arr);

    auto sub = span.last(2);
    EXPECT_EQ(2_size, sub.size());
    EXPECT_EQ(4, sub[0]);
    EXPECT_EQ(5, sub[1]);
}

TEST_F(SpanTest, MiddleSubspan)
{
    int arr[] = {1, 2, 3, 4, 5, 6};
    std::span<int> span(arr);

    auto sub = span.subspan(2, 3);
    EXPECT_EQ(3_size, sub.size());
    EXPECT_EQ(3, sub[0]);
    EXPECT_EQ(4, sub[1]);
    EXPECT_EQ(5, sub[2]);
}

// ------------------------------
// Fixed Extent Tests
// ------------------------------

TEST_F(SpanTest, FixedExtentProperties)
{
    int arr[] = {1, 2, 3, 4};
    std::span<int, 4> span(arr);

    EXPECT_EQ(4_size, span.size());
    EXPECT_EQ(4_size, span.extent);
    EXPECT_EQ(false, span.empty());

    static_assert(span.extent == 4);
}

TEST_F(SpanTest, DynamicExtentProperties)
{
    int arr[] = {1, 2, 3, 4, 5};
    std::span<int> span(arr);

    EXPECT_EQ(5_size, span.size());
    EXPECT_EQ(std::dynamic_extent, span.extent);
    EXPECT_EQ(false, span.empty());
}

// ------------------------------
// Complex Type Tests
// ------------------------------

TEST_F(SpanTest, ComplexTypes)
{
    struct TestStruct {
        int x;
        double y;

        bool operator==(const TestStruct &other) const { return x == other.x && y == other.y; }
    };

    TestStruct arr[] = {
        {1, 1.1},
        {2, 2.2},
        {3, 3.3}
    };

    std::span<TestStruct> span(arr);

    EXPECT_EQ(3_size, span.size());
    EXPECT_EQ(1, span[0].x);
    EXPECT_EQ(1.1, span[0].y);
    EXPECT_EQ(2, span[1].x);
    EXPECT_EQ(2.2, span[1].y);
    EXPECT_EQ(3, span[2].x);
    EXPECT_EQ(3.3, span[2].y);

    span[1].x = 42;
    EXPECT_EQ(42, arr[1].x);
}

// ------------------------------
// Span structure size
// ------------------------------

TEST_F(SpanTest, SpanStructureSize)
{
    EXPECT_EQ(16UL, sizeof(std::span<int>));
    EXPECT_EQ(8UL, sizeof(std::span<int, 5>));
}

// ------------------------------
// std::as_bytes, std::as_writable_bytes
// ------------------------------

TEST_F(SpanTest, AsBytes)
{
    int arr[]      = {288, 42, 1337, 4};
    auto byte_span = std::as_bytes(std::span<int>(arr));

    EXPECT_TRUE(std::is_const_v<decltype(byte_span)::element_type>);
    EXPECT_EQ(16_size, byte_span.size());
    EXPECT_EQ(32, byte_span[0]);
    EXPECT_EQ(1, byte_span[1]);
    EXPECT_EQ(0, byte_span[2]);
    EXPECT_EQ(0, byte_span[3]);
}

TEST_F(SpanTest, AsWritableBytes)
{
    int arr[]               = {1, 2, 3, 4};
    auto writable_byte_span = std::as_writable_bytes(std::span<int>(arr));

    EXPECT_FALSE(std::is_const_v<decltype(writable_byte_span)::element_type>);
    EXPECT_EQ(16_size, writable_byte_span.size());
    writable_byte_span[1] = 1;
    EXPECT_EQ(257, arr[0]);
}
