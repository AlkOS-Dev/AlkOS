/* internal includes */
#include <assert.h>
#include <string.h>
#include <array.hpp>
#include <data_structures/array_structures.hpp>
#include <test_module/test.hpp>

using namespace data_structures;

class ArrayTest : public TestGroupBase
{
};

// ------------------------------
// Basic Construction Tests
// ------------------------------

TEST_F(ArrayTest, DefaultConstructor)
{
    std::array<int, 5> arr{};
    EXPECT_EQ(5_size, arr.size());
    EXPECT_EQ(5_size, arr.max_size());
    EXPECT_EQ(false, arr.empty());
}

TEST_F(ArrayTest, BracedInitialization)
{
    std::array<int, 3> arr = {1, 2, 3};
    EXPECT_EQ(1, arr[0]);
    EXPECT_EQ(2, arr[1]);
    EXPECT_EQ(3, arr[2]);
}

TEST_F(ArrayTest, EmptyArray)
{
    std::array<int, 0> arr;
    EXPECT_EQ(0u, arr.size());
    EXPECT_EQ(true, arr.empty());
}

// ------------------------------
// Element Access Tests
// ------------------------------

TEST_F(ArrayTest, ElementAccess)
{
    std::array<int, 4> arr = {10, 20, 30, 40};

    EXPECT_EQ(10, arr[0]);
    EXPECT_EQ(20, arr[1]);
    EXPECT_EQ(30, arr[2]);
    EXPECT_EQ(40, arr[3]);

    EXPECT_EQ(10, arr.at(0));
    EXPECT_EQ(20, arr.at(1));
    EXPECT_EQ(30, arr.at(2));
    EXPECT_EQ(40, arr.at(3));

    EXPECT_EQ(10, arr.front());
    EXPECT_EQ(40, arr.back());

    EXPECT_EQ(10, *(arr.data()));
    EXPECT_EQ(20, *(arr.data() + 1));
}

TEST_F(ArrayTest, ConstElementAccess)
{
    const std::array<int, 4> arr = {10, 20, 30, 40};

    EXPECT_EQ(10, arr[0]);
    EXPECT_EQ(20, arr[1]);

    EXPECT_EQ(10, arr.at(0));
    EXPECT_EQ(20, arr.at(1));

    EXPECT_EQ(10, arr.front());
    EXPECT_EQ(40, arr.back());

    EXPECT_EQ(10, *(arr.data()));
    EXPECT_EQ(20, *(arr.data() + 1));
}

// ------------------------------
// Operations Tests
// ------------------------------

TEST_F(ArrayTest, FillOperation)
{
    std::array<int, 5> arr;
    arr.fill(42);

    EXPECT_EQ(42, arr[0]);
    EXPECT_EQ(42, arr[1]);
    EXPECT_EQ(42, arr[2]);
    EXPECT_EQ(42, arr[3]);
    EXPECT_EQ(42, arr[4]);
}

TEST_F(ArrayTest, SwapOperation)
{
    std::array<int, 3> arr1 = {1, 2, 3};
    std::array<int, 3> arr2 = {4, 5, 6};

    arr1.swap(arr2);

    EXPECT_EQ(4, arr1[0]);
    EXPECT_EQ(5, arr1[1]);
    EXPECT_EQ(6, arr1[2]);

    EXPECT_EQ(1, arr2[0]);
    EXPECT_EQ(2, arr2[1]);
    EXPECT_EQ(3, arr2[2]);

    std::swap(arr1, arr2);

    EXPECT_EQ(1, arr1[0]);
    EXPECT_EQ(2, arr1[1]);
    EXPECT_EQ(3, arr1[2]);

    EXPECT_EQ(4, arr2[0]);
    EXPECT_EQ(5, arr2[1]);
    EXPECT_EQ(6, arr2[2]);
}

// ------------------------------
// Iterator Tests
// ------------------------------

TEST_F(ArrayTest, IteratorAccess)
{
    std::array<int, 3> arr = {10, 20, 30};

    auto it = arr.begin();
    EXPECT_EQ(10, *it);
    ++it;
    EXPECT_EQ(20, *it);
    ++it;
    EXPECT_EQ(30, *it);
    ++it;
    EXPECT_EQ(arr.end(), it);

    const std::array<int, 3> &const_arr = arr;
    auto const_it                       = const_arr.begin();
    EXPECT_EQ(10, *const_it);
    ++const_it;
    EXPECT_EQ(20, *const_it);

    auto c_it = arr.cbegin();
    EXPECT_EQ(10, *c_it);
    ++c_it;
    EXPECT_EQ(20, *c_it);
}

TEST_F(ArrayTest, ReverseIteratorAccess)
{
    std::array<int, 3> arr = {10, 20, 30};

    auto r_it = arr.rbegin();
    EXPECT_EQ(arr.end(), r_it);
    EXPECT_EQ(arr.begin(), arr.rend());

    TODO_LIBCPP_COMPLIANCE
    // Since reverse iterators aren't fully implemented yet, we're just checking
    // that the basic methods exist and return the expected types
}

// ------------------------------
// Comparison Tests
// ------------------------------

TEST_F(ArrayTest, EqualityComparison)
{
    std::array<int, 3> arr1 = {1, 2, 3};
    std::array<int, 3> arr2 = {1, 2, 3};
    std::array<int, 3> arr3 = {3, 2, 1};

    EXPECT_EQ(true, arr1 == arr2);
    EXPECT_EQ(false, arr1 == arr3);
}

// ------------------------------
// std::get Tests
// ------------------------------

TEST_F(ArrayTest, GetFunction)
{
    std::array<int, 3> arr = {10, 20, 30};

    EXPECT_EQ(10, std::get<0>(arr));
    EXPECT_EQ(20, std::get<1>(arr));
    EXPECT_EQ(30, std::get<2>(arr));

    std::get<1>(arr) = 25;
    EXPECT_EQ(25, arr[1]);

    const std::array<int, 3> const_arr = {40, 50, 60};
    EXPECT_EQ(40, std::get<0>(const_arr));
    EXPECT_EQ(50, std::get<1>(const_arr));
    EXPECT_EQ(60, std::get<2>(const_arr));
}

// ------------------------------
// Complex Type Tests
// ------------------------------

TEST_F(ArrayTest, ComplexTypes)
{
    struct TestStruct {
        int x;
        double y;

        bool operator==(const TestStruct &other) const { return x == other.x && y == other.y; }

        bool operator!=(const TestStruct &other) const { return !(*this == other); }
    };

    std::array<TestStruct, 2> arr = {
        TestStruct{1, 1.1},
        TestStruct{2, 2.2}
    };

    EXPECT_EQ(1, arr[0].x);
    EXPECT_EQ(1.1, arr[0].y);
    EXPECT_EQ(2, arr[1].x);
    EXPECT_EQ(2.2, arr[1].y);

    TestStruct fill_value{5, 5.5};
    arr.fill(fill_value);

    EXPECT_EQ(5, arr[0].x);
    EXPECT_EQ(5.5, arr[0].y);
    EXPECT_EQ(5, arr[1].x);
    EXPECT_EQ(5.5, arr[1].y);
}

// ------------------------------
// StringArray Tests
// ------------------------------

TEST_F(ArrayTest, StringArrayConstruction)
{
    StringArray<5> str("hello");
    EXPECT_EQ(sizeof(StringArray<5>), 5u);
    EXPECT_EQ('h', str[0]);
    EXPECT_EQ('e', str[1]);
    EXPECT_EQ('l', str[2]);
    EXPECT_EQ('l', str[3]);
    EXPECT_EQ('o', str[4]);
}

TEST_F(ArrayTest, StringArrayShortString)
{
    StringArray<10> str("test");
    EXPECT_EQ('t', str[0]);
    EXPECT_EQ('e', str[1]);
    EXPECT_EQ('s', str[2]);
    EXPECT_EQ('t', str[3]);
}

TEST_F(ArrayTest, StringArrayEmptyString)
{
    StringArray<3> str("");
    EXPECT_EQ(3u, str.size());
}

TEST_F(ArrayTest, StringArrayGetSafeStr)
{
    StringArray<4> str("test");
    auto safe = str.GetSafeStr();
    EXPECT_EQ(5u, safe.size());  // kSize + 1
    EXPECT_EQ('\0', safe[4]);    // Null terminator
}

TEST_F(ArrayTest, StringArrayGetCStr)
{
    StringArray<5> str("hello");
    const char *cstr = str.GetCStr();
    EXPECT_EQ('h', cstr[0]);
    EXPECT_EQ('e', cstr[1]);
}
