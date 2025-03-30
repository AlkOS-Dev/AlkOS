#include <extensions/templates/multi_array.hpp>
#include <extensions/type_traits.hpp>
#include "test_module/test.hpp"

using namespace TemplateLib;

// Helper struct for testing non-fundamental types
struct TestPoint {
    int x = 0;
    int y = 0;
    bool operator==(const TestPoint& other) const { return x == other.x && y == other.y; }
    bool operator!=(const TestPoint& other) const { return !(*this == other); }
    TestPoint() = default;
    TestPoint(int x_val, int y_val) : x(x_val), y(y_val) {}
};

class MultiArrayTests : public TestGroupBase
{
    protected:
    template <size_t N>
    size_t CalculateExpectedFlatIndex(
        const std::array<size_t, N>& indices, const std::array<size_t, N>& dims,
        const std::array<size_t, N>& strides
    )
    {
        size_t flat_index = 0;
        for (size_t i = 0; i < N; ++i) {
            if (indices[i] >= dims[i]) {
                R_FAIL_ALWAYS("Index out of bounds in test logic");
            }
            flat_index += indices[i] * strides[i];
        }
        return flat_index;
    }
};

//------------------------------------------------------------------------------
// Test Group: Basic Construction and Metadata
//------------------------------------------------------------------------------

// Test: GetRank_ReturnsCorrectDimensionCount
// Given: MultiArrays of different ranks.
// When: Calling GetRank().
// Then: Returns the correct number of dimensions specified in the template parameters.
TEST_F(MultiArrayTests, GetRank_ReturnsCorrectDimensionCount)
{
    // Given
    MultiArray<int, 5> arr1d;
    MultiArray<float, 2, 3> arr2d;
    MultiArray<double, 4, 1, 7, 2> arr4d;

    // When / Then
    EXPECT_EQ(arr1d.GetRank(), 1u);
    EXPECT_EQ(arr2d.GetRank(), 2u);
    EXPECT_EQ(arr4d.GetRank(), 4u);
    // Constexpr check
    static_assert(MultiArray<int, 5>::GetRank() == 1u);
    static_assert(MultiArray<float, 2, 3>::GetRank() == 2u);
    static_assert(MultiArray<double, 4, 1, 7, 2>::GetRank() == 4u);
}

// Test: GetTotalSize_ReturnsCorrectProductOfDimensions
// Given: MultiArrays with various dimensions.
// When: Calling GetTotalSize().
// Then: Returns the correct total number of elements (product of all dimensions).
TEST_F(MultiArrayTests, GetTotalSize_ReturnsCorrectProductOfDimensions)
{
    // Given
    MultiArray<int, 5> arr1d;               // 5
    MultiArray<float, 2, 3> arr2d;          // 2 * 3 = 6
    MultiArray<double, 4, 1, 7, 2> arr4d;   // 4 * 1 * 7 * 2 = 56
    MultiArray<char, 1, 1, 1, 1, 1> arr5d;  // 1

    // When / Then (Use unsigned literal for comparison)
    EXPECT_EQ(arr1d.GetTotalSize(), 5u);
    EXPECT_EQ(arr2d.GetTotalSize(), 6u);
    EXPECT_EQ(arr4d.GetTotalSize(), 56u);
    EXPECT_EQ(arr5d.GetTotalSize(), 1u);

    static_assert(MultiArray<int, 5>::GetTotalSize() == 5u);
    static_assert(MultiArray<float, 2, 3>::GetTotalSize() == 6u);
    static_assert(MultiArray<double, 4, 1, 7, 2>::GetTotalSize() == 56u);
    static_assert(MultiArray<char, 1, 1, 1, 1, 1>::GetTotalSize() == 1u);
}

// Test: GetDims_ReturnsCorrectDimensionsArray
// Given: A MultiArray.
// When: Calling GetDims().
// Then: Returns a std::array containing the dimensions specified in the template parameters.
TEST_F(MultiArrayTests, GetDims_ReturnsCorrectDimensionsArray)
{
    // Given
    MultiArray<double, 4, 1, 7, 2> arr;
    constexpr std::array<size_t, 4> expected_dims = {4, 1, 7, 2};

    // When
    auto dims = arr.GetDims();

    // Then
    EXPECT_EQ(dims.size(), 4u);
    EXPECT_EQ(dims, expected_dims);
    static_assert(MultiArray<double, 4, 1, 7, 2>::GetDims() == expected_dims);
}

// Test: GetStrides_ReturnsCorrectStridesArrayForDefaultLayout
// Given: A MultiArray (assuming row-major layout).
// When: Calling GetStrides().
// Then: Returns a std::array containing the correct strides for each dimension.
TEST_F(MultiArrayTests, GetStrides_ReturnsCorrectStridesArrayForDefaultLayout)
{
    // Given
    MultiArray<int, 2, 3, 4> arr;  // Size: 2x3x4 = 24
    constexpr std::array<size_t, 3> expected_strides = {12, 4, 1};

    // When
    auto strides = arr.GetStrides();

    // Then
    EXPECT_EQ(strides.size(), 3u);  // Use unsigned literal
    EXPECT_EQ(strides, expected_strides);
    // Constexpr check
    static_assert(MultiArray<int, 2, 3, 4>::GetStrides() == expected_strides);

    // Given 1D
    MultiArray<float, 10> arr1d;
    constexpr std::array<size_t, 1> expected_strides_1d = {1};
    // When / Then
    EXPECT_EQ(arr1d.GetStrides(), expected_strides_1d);
    static_assert(MultiArray<float, 10>::GetStrides() == expected_strides_1d);

    // Given 4D with size 1 dimension
    MultiArray<double, 4, 1, 7, 2> arr4d;  // Sizes {4, 1, 7, 2}, Total = 56
    constexpr std::array<size_t, 4> expected_strides_4d = {14, 14, 2, 1};
    // When / Then
    EXPECT_EQ(arr4d.GetStrides(), expected_strides_4d);
    static_assert(MultiArray<double, 4, 1, 7, 2>::GetStrides() == expected_strides_4d);
}

// Test: GetData_NonConst_ReturnsModifiableArrayReference
// Given: A non-const MultiArray.
// When: Calling GetData().
// Then: Returns a non-const reference to the underlying std::array, allowing modification.
TEST_F(MultiArrayTests, GetData_NonConst_ReturnsModifiableArrayReference)
{
    // Given
    MultiArray<int, 2, 2> arr;

    // When
    auto& data_ref = arr.GetData();
    data_ref[1]    = 99;

    // Then
    EXPECT_EQ(arr(0, 1), 99);
    EXPECT_EQ(arr.GetData()[1], 99);
}

// Test: GetData_Const_ReturnsConstArrayReference
// Given: A const MultiArray.
// When: Calling GetData().
// Then: Returns a const reference to the underlying std::array, preventing modification.
TEST_F(MultiArrayTests, GetData_Const_ReturnsConstArrayReference)
{
    // Given
    MultiArray<int, 2, 2> arr_mut;
    arr_mut(0, 1)                    = 99;
    const MultiArray<int, 2, 2>& arr = arr_mut;

    // When
    const auto& data_ref = arr.GetData();

    // Then
    EXPECT_EQ(data_ref[1], 99);

    static_assert(
        std::is_const_v<std::remove_reference_t<decltype(arr.GetData())>>,
        "GetData on const object should return const ref to array"
    );
    using ExpectedElementType = const int&;
    static_assert(
        std::is_same_v<decltype(arr.GetData()[0]), ExpectedElementType>,
        "GetData()[0] on const object should return const T&"
    );
}

// Test: GetData_SizeMatchesTotalSize
// Given: A MultiArray.
// When: Calling GetData().size().
// Then: The size of the returned std::array matches GetTotalSize().
TEST_F(MultiArrayTests, GetData_SizeMatchesTotalSize)
{
    // Given
    MultiArray<int, 2, 3, 4> arr;  // Total size 24

    // When / Then
    EXPECT_EQ(arr.GetData().size(), 24u);
    EXPECT_EQ(arr.GetData().size(), arr.GetTotalSize());

    // Given
    MultiArray<char, 7> arr1d;
    // When / Then
    EXPECT_EQ(arr1d.GetData().size(), 7u);
    EXPECT_EQ(arr1d.GetData().size(), arr1d.GetTotalSize());
}

// ========================================================================== //
// Test Group: operator() Access (Non-const)
// ========================================================================== //

// Test: OperatorParens_NonConst_AccessAndModifyFirstElement
// Given: A non-const MultiArray (2D, 3D).
// When: Accessing and modifying the element at index (0, 0, ...) using operator().
// Then: The element is correctly modified and subsequent reads return the new value.
TEST_F(MultiArrayTests, OperatorParens_NonConst_AccessAndModifyFirstElement)
{
    // 2D
    MultiArray<int, 3, 4> arr2d;
    arr2d(0, 0) = 123;
    EXPECT_EQ(arr2d(0, 0), 123);
    EXPECT_EQ(arr2d.GetData()[0], 123);

    // 3D
    MultiArray<double, 2, 3, 2> arr3d;
    arr3d(0, 0, 0) = 3.14;
    EXPECT_EQ(arr3d(0, 0, 0), 3.14);
    EXPECT_EQ(arr3d.GetData()[0], 3.14);
}

// Test: OperatorParens_NonConst_AccessAndModifyLastElement
// Given: A non-const MultiArray (2D, 3D).
// When: Accessing and modifying the element at the last index using operator().
// Then: The element is correctly modified and subsequent reads return the new value.
TEST_F(MultiArrayTests, OperatorParens_NonConst_AccessAndModifyLastElement)
{
    // 2D
    MultiArray<int, 3, 4> arr2d;
    arr2d(2, 3) = 456;
    EXPECT_EQ(arr2d(2, 3), 456);
    EXPECT_EQ(arr2d.GetData()[arr2d.GetTotalSize() - 1], 456);

    // 3D
    MultiArray<double, 2, 3, 2> arr3d;
    arr3d(1, 2, 1) = -1.0;
    EXPECT_EQ(arr3d(1, 2, 1), -1.0);
    EXPECT_EQ(arr3d.GetData()[arr3d.GetTotalSize() - 1], -1.0);
}

// Test: OperatorParens_NonConst_AccessAndModifyMiddleElement
// Given: A non-const MultiArray (3D).
// When: Accessing and modifying an element in the middle using operator().
// Then: The element is correctly modified and subsequent reads return the new value.
TEST_F(MultiArrayTests, OperatorParens_NonConst_AccessAndModifyMiddleElement)
{
    // 3D
    MultiArray<int, 4, 5, 6> arr3d;
    arr3d(1, 2, 3) = 789;
    EXPECT_EQ(arr3d(1, 2, 3), 789);

    // Verify flat index calculation manually
    auto strides = arr3d.GetStrides();
    size_t expected_flat_index =
        1 * strides[0] + 2 * strides[1] + 3 * strides[2];  // 1*30 + 2*6 + 3*1 = 30 + 12 + 3 = 45
    EXPECT_EQ(arr3d.GetData()[expected_flat_index], 789);
}

// Test: OperatorParens_NonConst_AcceptsConvertibleIndexTypes
// Given: A non-const MultiArray.
// When: Accessing elements using operator() with index types convertible to size_t (e.g., int,
// short). Then: Access is successful.
TEST_F(MultiArrayTests, OperatorParens_NonConst_AcceptsConvertibleIndexTypes)
{
    MultiArray<int, 5, 5> arr;
    int i      = 1;
    short j    = 2;
    unsigned k = 3;
    long l     = 4;

    arr(i, j) = 12;
    EXPECT_EQ(arr(1, 2), 12);

    arr(k, l) = 34;
    EXPECT_EQ(arr(3, 4), 34);
}

// Test: OperatorParens_NonConst_1DArrayAccess
// Given: A non-const 1D MultiArray.
// When: Accessing and modifying elements using operator().
// Then: Elements are accessed and modified correctly.
TEST_F(MultiArrayTests, OperatorParens_NonConst_1DArrayAccess)
{
    MultiArray<int, 5> arr;
    arr(0) = 111;
    arr(4) = 555;
    EXPECT_EQ(arr(0), 111);
    EXPECT_EQ(arr(4), 555);
    EXPECT_EQ(arr.GetData()[0], 111);
    EXPECT_EQ(arr.GetData()[4], 555);
}

// Test: OperatorParens_NonConst_UserDefinedTypeAccess
// Given: A non-const MultiArray of a user-defined struct (TestPoint).
// When: Accessing and modifying elements using operator().
// Then: Elements (structs) are accessed and modified correctly.
TEST_F(MultiArrayTests, OperatorParens_NonConst_UserDefinedTypeAccess)
{
    MultiArray<TestPoint, 2, 2> arr;
    arr(0, 1)   = TestPoint(10, 20);
    arr(1, 0).x = 30;
    arr(1, 0).y = 40;

    EXPECT_EQ(arr(0, 1), TestPoint(10, 20));
    EXPECT_EQ(arr(1, 0).x, 30);
    EXPECT_EQ(arr(1, 0).y, 40);
    EXPECT_EQ(arr(1, 1), TestPoint(0, 0));
}

//------------------------------------------------------------------------------
// Test Group: operator() Access (Const)
//------------------------------------------------------------------------------

// Test: OperatorParens_Const_AccessFirstElement
// Given: A const MultiArray (2D, 3D).
// When: Accessing the element at index (0, 0, ...) using operator().
// Then: The correct element value is returned.
TEST_F(MultiArrayTests, OperatorParens_Const_AccessFirstElement)
{
    // 2D
    MultiArray<int, 3, 4> arr2d_mut;
    arr2d_mut(0, 0)                    = 123;
    const MultiArray<int, 3, 4>& arr2d = arr2d_mut;
    EXPECT_EQ(arr2d(0, 0), 123);

    // 3D
    MultiArray<double, 2, 3, 2> arr3d_mut;
    arr3d_mut(0, 0, 0)                       = 3.14;
    const MultiArray<double, 2, 3, 2>& arr3d = arr3d_mut;
    EXPECT_EQ(arr3d(0, 0, 0), 3.14);

    static_assert(
        std::is_const_v<std::remove_reference_t<decltype(arr2d(0, 0))>>,
        "const op() should return const&"
    );
}

// Test: OperatorParens_Const_AccessLastElement
// Given: A const MultiArray (2D, 3D).
// When: Accessing the element at the last index using operator().
// Then: The correct element value is returned.
TEST_F(MultiArrayTests, OperatorParens_Const_AccessLastElement)
{
    // 2D
    MultiArray<int, 3, 4> arr2d_mut;
    arr2d_mut(2, 3)                    = 456;
    const MultiArray<int, 3, 4>& arr2d = arr2d_mut;
    EXPECT_EQ(arr2d(2, 3), 456);

    // 3D
    MultiArray<double, 2, 3, 2> arr3d_mut;
    arr3d_mut(1, 2, 1)                       = -1.0;
    const MultiArray<double, 2, 3, 2>& arr3d = arr3d_mut;
    EXPECT_EQ(arr3d(1, 2, 1), -1.0);
}

// Test: OperatorParens_Const_AccessMiddleElement
// Given: A const MultiArray (3D).
// When: Accessing an element in the middle using operator().
// Then: The correct element value is returned.
TEST_F(MultiArrayTests, OperatorParens_Const_AccessMiddleElement)
{
    // 3D
    MultiArray<int, 4, 5, 6> arr3d_mut;
    arr3d_mut(1, 2, 3)                    = 789;
    const MultiArray<int, 4, 5, 6>& arr3d = arr3d_mut;
    EXPECT_EQ(arr3d(1, 2, 3), 789);
}

// Test: OperatorParens_Const_AcceptsConvertibleIndexTypes
// Given: A const MultiArray.
// When: Accessing elements using operator() with index types convertible to size_t (e.g., int,
// short). Then: Access is successful and returns the correct value.
TEST_F(MultiArrayTests, OperatorParens_Const_AcceptsConvertibleIndexTypes)
{
    MultiArray<int, 5, 5> arr_mut;
    arr_mut(1, 2)                    = 12;
    arr_mut(3, 4)                    = 34;
    const MultiArray<int, 5, 5>& arr = arr_mut;

    int i      = 1;
    short j    = 2;
    unsigned k = 3;
    long l     = 4;

    EXPECT_EQ(arr(i, j), 12);  // int, short
    EXPECT_EQ(arr(k, l), 34);  // unsigned, long
}

// Test: OperatorParens_Const_1DArrayAccess
// Given: A const 1D MultiArray.
// When: Accessing elements using operator().
// Then: Elements are accessed correctly.
TEST_F(MultiArrayTests, OperatorParens_Const_1DArrayAccess)
{
    MultiArray<int, 5> arr_mut;
    arr_mut(0)                    = 111;
    arr_mut(4)                    = 555;
    const MultiArray<int, 5>& arr = arr_mut;

    EXPECT_EQ(arr(0), 111);
    EXPECT_EQ(arr(4), 555);
}

// Test: OperatorParens_Const_UserDefinedTypeAccess
// Given: A const MultiArray of a user-defined struct (TestPoint).
// When: Accessing elements using operator().
// Then: Elements (structs) are accessed correctly (const access).
TEST_F(MultiArrayTests, OperatorParens_Const_UserDefinedTypeAccess)
{
    MultiArray<TestPoint, 2, 2> arr_mut;
    arr_mut(0, 1)                          = TestPoint(10, 20);
    arr_mut(1, 0)                          = TestPoint(30, 40);
    const MultiArray<TestPoint, 2, 2>& arr = arr_mut;

    EXPECT_EQ(arr(0, 1), TestPoint(10, 20));
    EXPECT_EQ(arr(1, 0).x, 30);
    EXPECT_EQ(arr(1, 0).y, 40);
}

//------------------------------------------------------------------------------
// Test Group: operator[] Access (Non-const)
//------------------------------------------------------------------------------

// Test: OperatorSquare_NonConst_AccessAndModifyFirstElement
// Given: A non-const MultiArray (2D, 3D).
// When: Accessing and modifying the element at index [0][0]... using operator[].
// Then: The element is correctly modified and subsequent reads return the new value.
TEST_F(MultiArrayTests, OperatorSquare_NonConst_AccessAndModifyFirstElement)
{
    // 2D
    MultiArray<int, 3, 4> arr2d;
    arr2d[0][0] = 123;
    EXPECT_EQ(arr2d[0][0], 123);
    EXPECT_EQ(arr2d(0, 0), 123);
    EXPECT_EQ(arr2d.GetData()[0], 123);

    // 3D
    MultiArray<double, 2, 3, 2> arr3d;
    arr3d[0][0][0] = 3.14;
    EXPECT_EQ(arr3d[0][0][0], 3.14);
    EXPECT_EQ(arr3d(0, 0, 0), 3.14);
    EXPECT_EQ(arr3d.GetData()[0], 3.14);
}

// Test: OperatorSquare_NonConst_AccessAndModifyLastElement
// Given: A non-const MultiArray (2D, 3D).
// When: Accessing and modifying the element at the last index using operator[].
// Then: The element is correctly modified and subsequent reads return the new value.
TEST_F(MultiArrayTests, OperatorSquare_NonConst_AccessAndModifyLastElement)
{
    // 2D
    MultiArray<int, 3, 4> arr2d;
    arr2d[2][3] = 456;
    EXPECT_EQ(arr2d[2][3], 456);
    EXPECT_EQ(arr2d(2, 3), 456);
    EXPECT_EQ(arr2d.GetData()[arr2d.GetTotalSize() - 1], 456);

    // 3D
    MultiArray<double, 2, 3, 2> arr3d;
    arr3d[1][2][1] = -1.0;
    EXPECT_EQ(arr3d[1][2][1], -1.0);
    EXPECT_EQ(arr3d(1, 2, 1), -1.0);
    EXPECT_EQ(arr3d.GetData()[arr3d.GetTotalSize() - 1], -1.0);
}

// Test: OperatorSquare_NonConst_AccessAndModifyMiddleElement
// Given: A non-const MultiArray (3D).
// When: Accessing and modifying an element in the middle using operator[].
// Then: The element is correctly modified and subsequent reads return the new value.
TEST_F(MultiArrayTests, OperatorSquare_NonConst_AccessAndModifyMiddleElement)
{
    // 3D
    MultiArray<int, 4, 5, 6> arr3d;
    arr3d[1][2][3] = 789;
    EXPECT_EQ(arr3d[1][2][3], 789);
    EXPECT_EQ(arr3d(1, 2, 3), 789);

    auto strides               = arr3d.GetStrides();                                // {30, 6, 1}
    size_t expected_flat_index = 1 * strides[0] + 2 * strides[1] + 3 * strides[2];  // 45
    EXPECT_EQ(arr3d.GetData()[expected_flat_index], 789);
}

// Test: OperatorSquare_NonConst_1DArrayAccess
// Given: A non-const 1D MultiArray.
// When: Accessing and modifying elements using operator[].
// Then: Elements are accessed and modified correctly.
TEST_F(MultiArrayTests, OperatorSquare_NonConst_1DArrayAccess)
{
    MultiArray<int, 5> arr;
    arr[0] = 111;
    arr[4] = 555;
    EXPECT_EQ(arr[0], 111);
    EXPECT_EQ(arr[4], 555);
    EXPECT_EQ(arr(0), 111);
    EXPECT_EQ(arr(4), 555);
}

// Test: OperatorSquare_NonConst_UserDefinedTypeAccess
// Given: A non-const MultiArray of a user-defined struct (TestPoint).
// When: Accessing and modifying elements using operator[].
// Then: Elements (structs) are accessed and modified correctly.
TEST_F(MultiArrayTests, OperatorSquare_NonConst_UserDefinedTypeAccess)
{
    MultiArray<TestPoint, 2, 2> arr;
    arr[0][1]   = TestPoint(10, 20);
    arr[1][0].x = 30;
    arr[1][0].y = 40;

    EXPECT_EQ(arr[0][1], TestPoint(10, 20));
    EXPECT_EQ(arr[1][0].x, 30);
    EXPECT_EQ(arr[1][0].y, 40);
    EXPECT_EQ(arr[1][1], TestPoint(0, 0));
    EXPECT_EQ(arr(0, 1), TestPoint(10, 20));
}

// Test: OperatorSquare_NonConst_IntermediateSliceType
// Given: A non-const MultiArray (e.g., 3D).
// When: Accessing using operator[] partially (e.g., arr[i]).
// Then: The returned type is an intermediate slice type, not the element type.
TEST_F(MultiArrayTests, OperatorSquare_NonConst_IntermediateSliceType)
{
    MultiArray<int, 2, 3, 4> arr;
    auto slice1 = arr[0];
    auto slice2 = arr[0][1];

    static_assert(
        !std::is_same_v<decltype(slice1), int&>, "Intermediate slice should not be element type"
    );
    static_assert(
        !std::is_same_v<decltype(slice2), int&>, "Intermediate slice should not be element type"
    );

    using ExpectedFinalType = int&;
    static_assert(
        std::is_same_v<decltype(arr[0][0][0]), ExpectedFinalType>,
        "Final access should yield element ref type"
    );
}

//------------------------------------------------------------------------------
// Test Group: operator[] Access (Const)
//------------------------------------------------------------------------------

// Test: OperatorSquare_Const_AccessFirstElement
// Given: A const MultiArray (2D, 3D).
// When: Accessing the element at index [0][0]... using operator[].
// Then: The correct element value is returned (const access).
TEST_F(MultiArrayTests, OperatorSquare_Const_AccessFirstElement)
{
    // 2D
    MultiArray<int, 3, 4> arr2d_mut;
    arr2d_mut(0, 0)                    = 123;
    const MultiArray<int, 3, 4>& arr2d = arr2d_mut;
    EXPECT_EQ(arr2d[0][0], 123);

    // 3D
    MultiArray<double, 2, 3, 2> arr3d_mut;
    arr3d_mut(0, 0, 0)                       = 3.14;
    const MultiArray<double, 2, 3, 2>& arr3d = arr3d_mut;
    EXPECT_EQ(arr3d[0][0][0], 3.14);

    using ExpectedFinalType2D = const int&;
    using ExpectedFinalType3D = const double&;
    static_assert(
        std::is_same_v<decltype(arr2d[0][0]), ExpectedFinalType2D>,
        "const op[] should return const T&"
    );
    static_assert(
        std::is_same_v<decltype(arr3d[0][0][0]), ExpectedFinalType3D>,
        "const op[] should return const T&"
    );
}

// Test: OperatorSquare_Const_AccessLastElement
// Given: A const MultiArray (2D, 3D).
// When: Accessing the element at the last index using operator[].
// Then: The correct element value is returned (const access).
TEST_F(MultiArrayTests, OperatorSquare_Const_AccessLastElement)
{
    // 2D
    MultiArray<int, 3, 4> arr2d_mut;
    arr2d_mut(2, 3)                    = 456;
    const MultiArray<int, 3, 4>& arr2d = arr2d_mut;
    EXPECT_EQ(arr2d[2][3], 456);

    // 3D
    MultiArray<double, 2, 3, 2> arr3d_mut;
    arr3d_mut(1, 2, 1)                       = -1.0;
    const MultiArray<double, 2, 3, 2>& arr3d = arr3d_mut;
    EXPECT_EQ(arr3d[1][2][1], -1.0);
}

// Test: OperatorSquare_Const_AccessMiddleElement
// Given: A const MultiArray (3D).
// When: Accessing an element in the middle using operator[].
// Then: The correct element value is returned (const access).
TEST_F(MultiArrayTests, OperatorSquare_Const_AccessMiddleElement)
{
    // 3D
    MultiArray<int, 4, 5, 6> arr3d_mut;
    arr3d_mut(1, 2, 3)                    = 789;
    const MultiArray<int, 4, 5, 6>& arr3d = arr3d_mut;
    EXPECT_EQ(arr3d[1][2][3], 789);
}

// Test: OperatorSquare_Const_1DArrayAccess
// Given: A const 1D MultiArray.
// When: Accessing elements using operator[].
// Then: Elements are accessed correctly (const access).
TEST_F(MultiArrayTests, OperatorSquare_Const_1DArrayAccess)
{
    MultiArray<int, 5> arr_mut;
    arr_mut(0)                    = 111;
    arr_mut(4)                    = 555;
    const MultiArray<int, 5>& arr = arr_mut;

    EXPECT_EQ(arr[0], 111);
    EXPECT_EQ(arr[4], 555);
}

// Test: OperatorSquare_Const_UserDefinedTypeAccess
// Given: A const MultiArray of a user-defined struct (TestPoint).
// When: Accessing elements using operator[].
// Then: Elements (structs) are accessed correctly (const access).
TEST_F(MultiArrayTests, OperatorSquare_Const_UserDefinedTypeAccess)
{
    MultiArray<TestPoint, 2, 2> arr_mut;
    arr_mut(0, 1)                          = TestPoint(10, 20);
    arr_mut(1, 0)                          = TestPoint(30, 40);
    const MultiArray<TestPoint, 2, 2>& arr = arr_mut;

    EXPECT_EQ(arr[0][1], TestPoint(10, 20));
    EXPECT_EQ(arr[1][0].x, 30);
    EXPECT_EQ(arr[1][0].y, 40);
}

// Test: OperatorSquare_Const_IntermediateSliceType
// Given: A const MultiArray (e.g., 3D).
// When: Accessing using operator[] partially (e.g., arr[i]).
// Then: The returned type is a const intermediate slice type, not the element type.
TEST_F(MultiArrayTests, OperatorSquare_Const_IntermediateSliceType)
{
    MultiArray<int, 2, 3, 4> arr_mut;
    const MultiArray<int, 2, 3, 4>& arr = arr_mut;
    auto slice1                         = arr[0];
    auto slice2                         = arr[0][1];

    static_assert(
        !std::is_same_v<decltype(slice1), const int&>,
        "Const intermediate slice should not be element type"
    );
    static_assert(
        !std::is_same_v<decltype(slice2), const int&>,
        "Const intermediate slice should not be element type"
    );

    using ExpectedFinalType = const int&;
    static_assert(
        std::is_same_v<decltype(arr[0][0][0]), ExpectedFinalType>,
        "Final const access should yield const element ref type"
    );
}

//------------------------------------------------------------------------------
// Test Group: Consistency between operator() and operator[]
//------------------------------------------------------------------------------

// Test: AccessConsistency_OperatorsYieldSameElement_NonConst
// Given: A non-const MultiArray (e.g., 3D).
// When: Accessing the same element using operator() and operator[].
// Then: Both operators yield a reference to the same element in memory.
TEST_F(MultiArrayTests, AccessConsistency_OperatorsYieldSameElement_NonConst)
{
    MultiArray<int, 2, 3, 4> arr;
    int i = 1, j = 1, k = 2;

    int& ref_parens = arr(i, j, k);
    int& ref_square = arr[i][j][k];

    ref_parens = 55;
    EXPECT_EQ(ref_square, 55);
    ref_square = 66;
    EXPECT_EQ(ref_parens, 66);
}

// Test: AccessConsistency_OperatorsYieldSameElement_Const
// Given: A const MultiArray (e.g., 3D).
// When: Accessing the same element using operator() and operator[].
// Then: Both operators yield a const reference to the same element in memory.
TEST_F(MultiArrayTests, AccessConsistency_OperatorsYieldSameElement_Const)
{
    MultiArray<int, 2, 3, 4> arr_mut;
    int i = 1, j = 1, k = 2;
    arr_mut(i, j, k)                    = 77;
    const MultiArray<int, 2, 3, 4>& arr = arr_mut;

    // Get const references using both methods
    const int& ref_parens = arr(i, j, k);
    const int& ref_square = arr[i][j][k];

    // Check they point to the same address
    EXPECT_EQ(&ref_parens, &ref_square);

    // Check value consistency
    EXPECT_EQ(ref_parens, 77);
    EXPECT_EQ(ref_square, 77);
}

//------------------------------------------------------------------------------
// Test Group: Interaction with GetData()
//------------------------------------------------------------------------------

// Test: GetDataInteraction_ModifyViaGetData_ReflectedInOperators
// Given: A MultiArray.
// When: Modifying an element directly in the array returned by GetData().
// Then: Accessing the same element via operator() and operator[] reflects the change.
TEST_F(MultiArrayTests, GetDataInteraction_ModifyViaGetData_ReflectedInOperators)
{
    MultiArray<int, 2, 3> arr;
    int i = 1, j = 1;
    auto strides      = arr.GetStrides();
    size_t flat_index = i * strides[0] + j * strides[1];

    // When
    arr.GetData()[flat_index] = 111;

    // Then
    EXPECT_EQ(arr(i, j), 111);
    EXPECT_EQ(arr[i][j], 111);
}

// Test: GetDataInteraction_ModifyViaOperators_ReflectedInGetData
// Given: A MultiArray.
// When: Modifying an element using operator() or operator[].
// Then: Accessing the corresponding element in the array returned by GetData() reflects the change.
TEST_F(MultiArrayTests, GetDataInteraction_ModifyViaOperators_ReflectedInGetData)
{
    MultiArray<int, 2, 3> arr;
    int i = 1, j = 1;
    auto strides      = arr.GetStrides();
    size_t flat_index = i * strides[0] + j * strides[1];

    // When (using operator())
    arr(i, j) = 222;
    // Then
    EXPECT_EQ(arr.GetData()[flat_index], 222);

    // When (using operator[])
    arr[0][1]            = 333;
    size_t flat_index_01 = 0 * strides[0] + 1 * strides[1];
    // Then
    EXPECT_EQ(arr.GetData()[flat_index_01], 333);
}

//------------------------------------------------------------------------------
// Test Group: Constexpr Evaluation
//------------------------------------------------------------------------------

// Test: Constexpr_MetadataFunctionsAreConstexpr
// Given: MultiArray type.
// When: Calling metadata functions in a constexpr context.
// Then: The functions evaluate correctly at compile time.
TEST_F(MultiArrayTests, Constexpr_MetadataFunctionsAreConstexpr)
{
    constexpr size_t D1 = 2, D2 = 3, D3 = 4;
    using TestArr = MultiArray<int, D1, D2, D3>;

    static_assert(TestArr::GetRank() == 3u, "");
    static_assert(TestArr::GetTotalSize() == D1 * D2 * D3, "");
    constexpr auto dims = TestArr::GetDims();
    static_assert(dims[0] == D1 && dims[1] == D2 && dims[2] == D3, "");
    constexpr auto strides = TestArr::GetStrides();
    static_assert(strides[0] == D2 * D3 && strides[1] == D3 && strides[2] == 1, "");
}

// Test: Constexpr_DefaultConstructorIsConstexpr
// Given: MultiArray type.
// When: Default constructing in a constexpr context.
// Then: Construction is successful at compile time.
TEST_F(MultiArrayTests, Constexpr_DefaultConstructorIsConstexpr)
{
    constexpr auto test = []() constexpr {
        MultiArray<int, 2, 2> arr;
        return arr.GetTotalSize();
    };
    constexpr size_t test_size = test();
    static_assert(test_size == 4, "");
}

// Test: Constexpr_OperatorParensConstAccessIsConstexpr (Requires C++20 for std::array constexpr
// access) Given: A constexpr MultiArray. When: Accessing elements using const operator() in a
// constexpr context. Then: Access is successful and returns the correct value at compile time.
TEST_F(MultiArrayTests, Constexpr_OperatorParensConstAccessIsConstexpr)
{
    constexpr MultiArray<int, 2, 3> arr = []() constexpr {
        MultiArray<int, 2, 3> a;

        a(0, 0) = 10;
        a(1, 2) = 20;
        return a;
    }();

    static_assert(arr(0, 0) == 10, "");
    static_assert(arr(1, 2) == 20, "");
    static_assert(arr(0, 1) == 0, "");
}

//------------------------------------------------------------------------------
// Test Group: Edge Cases
//------------------------------------------------------------------------------

// Test: EdgeCase_DimensionOfSizeOne
// Given: A MultiArray with one or more dimensions of size 1.
// When: Accessing elements using operators.
// Then: Access is successful and behaves correctly according to strides.
TEST_F(MultiArrayTests, EdgeCase_DimensionOfSizeOne)
{
    MultiArray<int, 4, 1, 3> arr;

    arr(0, 0, 0) = 1;
    arr(1, 0, 1) = 2;
    arr(3, 0, 2) = 3;

    EXPECT_EQ(arr[0][0][0], 1);
    EXPECT_EQ(arr[1][0][1], 2);
    EXPECT_EQ(arr[3][0][2], 3);

    // Check underlying data based on expected flat indices
    // (0,0,0) -> 0*3 + 0*3 + 0*1 = 0
    // (1,0,1) -> 1*3 + 0*3 + 1*1 = 4
    // (3,0,2) -> 3*3 + 0*3 + 2*1 = 11
    EXPECT_EQ(arr.GetData()[0], 1);
    EXPECT_EQ(arr.GetData()[4], 2);
    EXPECT_EQ(arr.GetData()[11], 3);
}

// Test: EdgeCase_OnlyOneElementTotal
// Given: A MultiArray with total size 1 (e.g., , <1,1>, <1,1,1>).
// When: Accessing the single element.
// Then: Access is successful.
TEST_F(MultiArrayTests, EdgeCase_OnlyOneElementTotal)
{
    // 1D
    MultiArray<int, 1> arr1d;
    arr1d(0) = 10;
    EXPECT_EQ(arr1d(0), 10);
    EXPECT_EQ(arr1d[0], 10);
    EXPECT_EQ(arr1d.GetData()[0], 10);
    EXPECT_EQ(arr1d.GetTotalSize(), 1u);

    // 3D
    MultiArray<double, 1, 1, 1> arr3d;
    arr3d(0, 0, 0) = 3.14;
    EXPECT_EQ(arr3d(0, 0, 0), 3.14);
    EXPECT_EQ(arr3d[0][0][0], 3.14);
    EXPECT_EQ(arr3d.GetData()[0], 3.14);
    EXPECT_EQ(arr3d.GetTotalSize(), 1u);
}

// Test: TypeSupport_WorksWithUserType // Renamed from StdString
// Given: A MultiArray templated with TestPoint.
// When: Creating, accessing, and modifying elements.
// Then: Behaves correctly for default construction, assignment, and access.
TEST_F(MultiArrayTests, TypeSupport_WorksWithUserType)
{
    MultiArray<TestPoint, 2, 2> arr;
    arr(0, 0) = TestPoint(1, 1);
    arr[0][1] = TestPoint(2, 2);
    arr[1][0] = TestPoint(3, 3);
    arr(1, 1) = TestPoint(4, 4);

    EXPECT_EQ(arr[0][0], TestPoint(1, 1));
    EXPECT_EQ(arr(0, 1), TestPoint(2, 2));
    EXPECT_EQ(arr(1, 0), TestPoint(3, 3));
    EXPECT_EQ(arr[1][1], TestPoint(4, 4));
}

//------------------------------------------------------------------------------
// Test Group: Construction (Advanced) & Assignment
//------------------------------------------------------------------------------

// Test: Constructor_InitializerList_InitializesCorrectly
// Given: An initializer list matching the total size.
// When: Creating a MultiArray using the initializer list constructor.
// Then: The elements are initialized in flat, row-major order from the list.
TEST_F(MultiArrayTests, Constructor_InitializerList_InitializesCorrectly)
{
    // Given
    MultiArray<int, 2, 3> arr_int({1, 2, 3, 4, 5, 6});

    // Then
    EXPECT_EQ(arr_int(0, 0), 1);
    EXPECT_EQ(arr_int(0, 1), 2);
    EXPECT_EQ(arr_int(0, 2), 3);
    EXPECT_EQ(arr_int(1, 0), 4);
    EXPECT_EQ(arr_int(1, 1), 5);
    EXPECT_EQ(arr_int(1, 2), 6);

    MultiArray<TestPoint, 1, 4> arr_point({
        TestPoint{1, 1},
        TestPoint{2, 2},
        TestPoint{3, 3},
        TestPoint{4, 4}
    });
    EXPECT_EQ(arr_point(0, 0), TestPoint(1, 1));
    EXPECT_EQ(arr_point(0, 3), TestPoint(4, 4));
}

// Test: Constructor_FillValue_InitializesAllElements
// Given: A fill value.
// When: Creating a MultiArray using the fill value constructor.
// Then: All elements in the MultiArray are initialized to the fill value.
TEST_F(MultiArrayTests, Constructor_FillValue_InitializesAllElements)
{
    // Given
    constexpr int fill_val_int = 99;
    MultiArray<int, 2, 2> arr_int(fill_val_int);

    // Then
    EXPECT_EQ(arr_int(0, 0), fill_val_int);
    EXPECT_EQ(arr_int(0, 1), fill_val_int);
    EXPECT_EQ(arr_int(1, 0), fill_val_int);
    EXPECT_EQ(arr_int(1, 1), fill_val_int);

    // Test with user-defined type
    TestPoint fill_val_point(123, 456);
    MultiArray<TestPoint, 1, 3> arr_point(fill_val_point);
    EXPECT_EQ(arr_point(0, 0), fill_val_point);
    EXPECT_EQ(arr_point(0, 1), fill_val_point);
    EXPECT_EQ(arr_point(0, 2), fill_val_point);
}

// Test: Constructor_Generator_InitializesBasedOnIndex
// Given: A generator function that takes multi-dimensional indices.
// When: Creating a MultiArray using the generator constructor.
// Then: Each element is initialized by calling the generator with its specific multi-dimensional
// index.
TEST_F(MultiArrayTests, Constructor_Generator_InitializesBasedOnIndex)
{
    // Given
    auto generator = [](const std::array<size_t, 2>& indices) {
        // Example: value = row * 10 + col
        return static_cast<int>(indices[0] * 10 + indices[1]);
    };

    // When
    MultiArray<int, 3, 4> arr(generator);

    // Then
    EXPECT_EQ(arr(0, 0), 0);   // 0*10 + 0
    EXPECT_EQ(arr(0, 1), 1);   // 0*10 + 1
    EXPECT_EQ(arr(1, 0), 10);  // 1*10 + 0
    EXPECT_EQ(arr(2, 3), 23);  // 2*10 + 3

    // Constexpr test
    constexpr auto generator_constexpr = [](const std::array<size_t, 2>& indices) {
        return indices[0] + indices[1];
    };
    constexpr MultiArray<size_t, 2, 2> arr_constexpr(generator_constexpr);
    static_assert(arr_constexpr(0, 0) == 0, "Constexpr generator failed");
    static_assert(arr_constexpr(0, 1) == 1, "Constexpr generator failed");
    static_assert(arr_constexpr(1, 0) == 1, "Constexpr generator failed");
    static_assert(arr_constexpr(1, 1) == 2, "Constexpr generator failed");
}

// Test: Constructor_Copy_CreatesIndependentCopy
// Given: An existing MultiArray initialized with some data.
// When: Creating a new MultiArray using the copy constructor.
// Then: The new array has the same dimensions and a deep copy of the data,
//       and modifying the new array does not affect the original.
TEST_F(MultiArrayTests, Constructor_Copy_CreatesIndependentCopy)
{
    // Given
    MultiArray<int, 2, 2> original(0);  // Fill with 0
    original(0, 1) = 55;
    original(1, 0) = 66;

    // When
    MultiArray<int, 2, 2> copy = original;  // Copy construction

    // Then
    // 1. Check metadata matches
    EXPECT_EQ(copy.GetRank(), original.GetRank());
    EXPECT_EQ(copy.GetTotalSize(), original.GetTotalSize());
    EXPECT_EQ(copy.GetDims(), original.GetDims());

    // 2. Check data is copied
    EXPECT_EQ(copy(0, 0), 0);
    EXPECT_EQ(copy(0, 1), 55);
    EXPECT_EQ(copy(1, 0), 66);
    EXPECT_EQ(copy(1, 1), 0);
    EXPECT_NEQ(&copy.GetData()[0], &original.GetData()[0]);  // Ensure different memory

    copy(0, 1) = 77;
    EXPECT_EQ(copy(0, 1), 77);
    EXPECT_EQ(original(0, 1), 55);  // Original should be unchanged
}

// Test: Constructor_Move_TransfersData
// Given: An existing MultiArray initialized with some data.
// When: Creating a new MultiArray using the move constructor.
// Then: The new array acquires the data from the original, and the original
//       is left in a valid (but likely default/empty) state.
TEST_F(MultiArrayTests, Constructor_Move_TransfersData)
{
    // Given
    MultiArray<TestPoint, 1, 3> source;
    source(0, 0) = TestPoint(1, 1);
    source(0, 1) = TestPoint(2, 2);
    source(0, 2) = TestPoint(3, 3);

    // When
    MultiArray<TestPoint, 1, 3> target = std::move(source);  // Move construction

    // Then
    // 1. Check metadata matches
    EXPECT_EQ(target.GetRank(), 2u);
    EXPECT_EQ(target.GetTotalSize(), 3u);
    EXPECT_EQ(target.GetDims(), (std::array<size_t, 2>{1, 3}));

    // 2. Check data is moved to target
    EXPECT_EQ(target(0, 0), TestPoint(1, 1));
    EXPECT_EQ(target(0, 1), TestPoint(2, 2));
    EXPECT_EQ(target(0, 2), TestPoint(3, 3));
}

// Test: OperatorAssign_Copy_CreatesIndependentCopy
// Given: Two existing MultiArrays, one initialized (source), one default (target).
// When: Assigning the source to the target using the copy assignment operator.
// Then: The target array gets a deep copy of the source data, and modifying
//       the target does not affect the source. Self-assignment is handled correctly.
TEST_F(MultiArrayTests, OperatorAssign_Copy_CreatesIndependentCopy)
{
    // Given
    MultiArray<int, 2, 2> source(0);
    source(0, 1) = 55;
    source(1, 0) = 66;
    MultiArray<int, 2, 2> target(99);  // Initial different value

    // When
    target = source;  // Copy assignment

    // Then
    // 1. Check data is copied
    EXPECT_EQ(target(0, 0), 0);
    EXPECT_EQ(target(0, 1), 55);
    EXPECT_EQ(target(1, 0), 66);
    EXPECT_EQ(target(1, 1), 0);
    EXPECT_NEQ(&target.GetData()[0], &source.GetData()[0]);

    // 2. Check independence
    target(0, 1) = 77;
    EXPECT_EQ(target(0, 1), 77);
    EXPECT_EQ(source(0, 1), 55);  // Source should be unchanged

    // 3. Check self-assignment
    int original_val_01 = source(0, 1);
    int original_val_10 = source(1, 0);
    source              = source;  // Self-assignment
    EXPECT_EQ(source(0, 1), original_val_01);
    EXPECT_EQ(source(1, 0), original_val_10);
}

// Test: OperatorAssign_Move_TransfersData
// Given: Two existing MultiArrays, one initialized (source), one default (target).
// When: Assigning the source to the target using the move assignment operator.
// Then: The target array acquires the data from the source, and the source
//       is left in a valid state. Self-assignment is handled correctly.
TEST_F(MultiArrayTests, OperatorAssign_Move_TransfersData)
{
    // Given
    MultiArray<TestPoint, 1, 3> source;
    source(0, 0) = TestPoint(1, 1);
    source(0, 1) = TestPoint(2, 2);
    source(0, 2) = TestPoint(3, 3);

    MultiArray<TestPoint, 1, 3> target;
    target(0, 0) = TestPoint(9, 9);  // Initial different value

    // When
    target = std::move(source);  // Move assignment

    // Then
    // 1. Check data is moved to target
    EXPECT_EQ(target(0, 0), TestPoint(1, 1));
    EXPECT_EQ(target(0, 1), TestPoint(2, 2));
    EXPECT_EQ(target(0, 2), TestPoint(3, 3));

    // 3. Check self-assignment
    MultiArray<int, 2, 2> self_move_arr;
    self_move_arr(1, 1) = 123;
    self_move_arr       = std::move(self_move_arr);
    EXPECT_EQ(self_move_arr(1, 1), 123);
}
