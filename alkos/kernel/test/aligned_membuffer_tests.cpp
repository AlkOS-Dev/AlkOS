#include <extensions/internal/aligned_membuffer.hpp>
#include <extensions/type_traits.hpp>
#include <test_module/test.hpp>

using namespace internal;

class AlignedMemoryBufferTest : public TestGroupBase
{
};

// Test: GetAddress_GetPtr_GivenNonConstAndConst_ReturnsConsistentAddress
// Given: A default constructed AlignedMemoryBuffer for type int.
// When: Retrieving the underlying address using both non-const and const getters.
// Then: The addresses obtained from GetAddress() and GetPtr() are consistent.
TEST_F(AlignedMemoryBufferTest, GetAddress_GetPtr_GivenNonConstAndConst_ReturnsConsistentAddress)
{
    // Given
    AlignedMemoryBuffer<int> buffer;

    // When (non-const)
    void* addr = buffer.GetAddress();
    int* ptr   = buffer.GetPtr();

    // Then
    EXPECT_EQ(addr, static_cast<void*>(ptr));

    // Given (const view)
    const AlignedMemoryBuffer<int>& const_buffer = buffer;

    // When (const)
    const void* const_addr = const_buffer.GetAddress();
    const int* const_ptr   = const_buffer.GetPtr();

    // Then
    EXPECT_EQ(const_addr, static_cast<const void*>(const_ptr));
}

// Test: GetPtr_GivenFundamentalType_ReturnsAlignedPointer
// Given: A default constructed AlignedMemoryBuffer for type int.
// When: Retrieving the pointer using GetPtr().
// Then: The pointer should be properly aligned to the alignment of int.
TEST_F(AlignedMemoryBufferTest, GetPtr_GivenFundamentalType_ReturnsAlignedPointer)
{
    // Given
    AlignedMemoryBuffer<int> buffer;

    // When
    int* ptr = buffer.GetPtr();

    // Then
    uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
    EXPECT_EQ(addr % alignof(int), 0);
}

// Test: GetPtr_GivenUserDefinedType_ReturnsAlignedPointer
// Given: A user-defined struct (TestStruct) and a default constructed AlignedMemoryBuffer for it.
// When: Retrieving the pointer using GetPtr().
// Then: The pointer should be non-null and properly aligned to the alignment requirements of
// TestStruct.
TEST_F(AlignedMemoryBufferTest, GetPtr_GivenUserDefinedType_ReturnsAlignedPointer)
{
    // Given: A non-trivial user-defined type.
    struct alignas(128) TestStruct {
        int a;
        double b;
    };
    // And: A default constructed AlignedMemoryBuffer for TestStruct.
    AlignedMemoryBuffer<TestStruct> buffer;

    // When
    TestStruct* ptr = buffer.GetPtr();

    // Then
    EXPECT_NOT_NULL(ptr);
    uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
    EXPECT_EQ(addr % alignof(TestStruct), 0);
}

// Test: GetPtr_GivenBuffer_ReturnsNonNullPointerAndSufficientStorageSize
// Given: A default constructed AlignedMemoryBuffer for type int.
// When: Retrieving the pointer using GetPtr().
// Then: The pointer is non-null and the storage is at least as large as sizeof(int).
TEST_F(AlignedMemoryBufferTest, GetPtr_GivenBuffer_ReturnsNonNullPointerAndSufficientStorageSize)
{
    // Given
    AlignedMemoryBuffer<int> buffer;

    // When
    int* ptr = buffer.GetPtr();

    // Then
    EXPECT_NOT_NULL(ptr);
    EXPECT_GE(sizeof(buffer), sizeof(int));
}

// Test: GetPtr_GivenBuffer_ModificationReflectsInConstView
// Given: A default constructed AlignedMemoryBuffer for type double and its const view.
// When: Modifying the value through the non-const pointer.
// Then: The updated value is correctly reflected when accessed via the const pointer.
TEST_F(AlignedMemoryBufferTest, GetPtr_GivenBuffer_ModificationReflectsInConstView)
{
    // Given
    AlignedMemoryBuffer<double> buffer;
    const AlignedMemoryBuffer<double>& const_buffer = buffer;

    // When
    *buffer.GetPtr() = 3.1415;

    // Then
    EXPECT_EQ(*const_buffer.GetPtr(), 3.1415);
}
