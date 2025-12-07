#include <algorithm.hpp>
#include <array.hpp>
#include <data_structures/atomic_cyclic_buffer.hpp>
#include <span.hpp>
#include <test_module/test.hpp>

using namespace data_structures;

// --------------------------------------------------------------------------------
// Helper Types
// --------------------------------------------------------------------------------

struct MoveTracker {
    int id = -1;

    MoveTracker() = default;
    explicit MoveTracker(int v) : id(v) {}

    MoveTracker(const MoveTracker &)            = delete;
    MoveTracker &operator=(const MoveTracker &) = delete;

    MoveTracker(MoveTracker &&other) noexcept : id(other.id) { other.id = -1; }

    MoveTracker &operator=(MoveTracker &&other) noexcept
    {
        if (this != &other) {
            id       = other.id;
            other.id = -1;
        }
        return *this;
    }

    bool IsValid() const { return id != -1; }
};

// --------------------------------------------------------------------------------
// Test Fixture
// --------------------------------------------------------------------------------

class AtomicCyclicBufferTest : public TestGroupBase
{
    protected:
    static constexpr size_t kSize = 16;
    AtomicCyclicBuffer<int, kSize> buffer_int;
};

// --------------------------------------------------------------------------------
// Constructor & Status Tests
// --------------------------------------------------------------------------------

TEST_F(AtomicCyclicBufferTest, Constructor_GivenDefault_InitializesEmptyState)
{
    EXPECT_TRUE(buffer_int.IsEmpty());
    EXPECT_FALSE(buffer_int.IsFull());
    EXPECT_EQ(0_size, buffer_int.Count());
    EXPECT_EQ(kSize, buffer_int.Capacity());
}

TEST_F(AtomicCyclicBufferTest, Capacity_GivenTemplateParam_ReturnsExactSize)
{
    AtomicCyclicBuffer<float, 32> buf;
    EXPECT_EQ(32_size, buf.Capacity());
}

// --------------------------------------------------------------------------------
// Write (Copy) Tests
// --------------------------------------------------------------------------------

TEST_F(AtomicCyclicBufferTest, WriteCopy_GivenEmptyBufferAndData_WritesSuccessfully)
{
    std::array<int, 4> data = {1, 2, 3, 4};

    auto count = buffer_int.Write(std::span<const int>(data));

    EXPECT_EQ(4_size, count);
    EXPECT_EQ(4_size, buffer_int.Count());
    EXPECT_FALSE(buffer_int.IsEmpty());
}

TEST_F(AtomicCyclicBufferTest, WriteCopy_GivenFullBuffer_ReturnsZero)
{
    // Fill buffer
    std::array<int, kSize> filler;
    filler.fill(100);
    buffer_int.Write(std::span<const int>(filler));

    ASSERT_TRUE(buffer_int.IsFull());

    // Attempt write
    int val    = 99;
    auto count = buffer_int.Write(std::span<const int>(&val, 1));

    EXPECT_EQ(0_size, count);
    EXPECT_EQ(kSize, buffer_int.Count());
}

TEST_F(AtomicCyclicBufferTest, WriteCopy_GivenPartialSpace_WritesOnlyAvailableCapacity)
{
    // Fill 14 out of 16
    std::array<int, kSize - 2> filler;
    filler.fill(10);
    buffer_int.Write(std::span<const int>(filler));

    // Try to write 5
    std::array<int, 5> input = {1, 2, 3, 4, 5};
    auto count               = buffer_int.Write(std::span<const int>(input));

    // Should write 2
    EXPECT_EQ(2_size, count);
    EXPECT_TRUE(buffer_int.IsFull());

    // Validate content via read
    std::array<int, kSize> output;
    buffer_int.Read(std::span<int>(output));

    EXPECT_EQ(1, output[kSize - 2]);
    EXPECT_EQ(2, output[kSize - 1]);
}

TEST_F(AtomicCyclicBufferTest, WriteCopy_GivenEmptySpan_DoesNothing)
{
    auto count = buffer_int.Write(std::span<const int>{});
    EXPECT_EQ(0_size, count);
    EXPECT_EQ(0_size, buffer_int.Count());
}

TEST_F(AtomicCyclicBufferTest, WriteCopy_GivenWrappedIndexScenario_WritesCorrectlyIntoBeginning)
{
    // Fill buffer entirely [0...15]
    std::array<int, kSize> data;
    for (size_t i = 0; i < kSize; ++i) data[i] = i;
    buffer_int.Write(std::span<const int>(data));

    // Read 4 items to create space at 'head' [gap, gap, gap, gap, 4...15]
    std::array<int, 4> garbage;
    buffer_int.Read(std::span<int>(garbage));

    // Write 4 items [100...103], wrapping around internal buffer
    std::array<int, 4> new_data = {100, 101, 102, 103};
    auto w_count                = buffer_int.Write(std::span<const int>(new_data));

    EXPECT_EQ(4_size, w_count);
    EXPECT_TRUE(buffer_int.IsFull());

    // Verify Read integrity
    std::array<int, kSize> result;
    auto r_count = buffer_int.Read(std::span<int>(result));

    EXPECT_EQ(kSize, r_count);
    // First part: 4..15
    for (size_t i = 0; i < kSize - 4; ++i) {
        EXPECT_EQ(static_cast<int>(i + 4), result[i]);
    }
    // Second part: 100..103
    EXPECT_EQ(100, result[kSize - 4]);
    EXPECT_EQ(103, result[kSize - 1]);
}

// --------------------------------------------------------------------------------
// Write (Move) Tests
// --------------------------------------------------------------------------------

TEST_F(AtomicCyclicBufferTest, WriteMove_GivenMoveOnlyType_MovesDataIntoBuffer)
{
    AtomicCyclicBuffer<MoveTracker, 8> move_buf;
    std::array<MoveTracker, 2> inputs;
    // Construct in place is hard with arrays without default ctor being MoveTracker()
    // We used default ctor in struct, so we are good.
    // Re-assign with valid values
    inputs[0] = MoveTracker(10);
    inputs[1] = MoveTracker(20);

    auto count = move_buf.Write(std::span<MoveTracker>(inputs));

    EXPECT_EQ(2_size, count);
    // Check if source moved-from
    EXPECT_FALSE(inputs[0].IsValid());
    EXPECT_FALSE(inputs[1].IsValid());

    // Read check
    std::array<MoveTracker, 2> outputs;
    move_buf.Read(std::span<MoveTracker>(outputs));

    EXPECT_EQ(10, outputs[0].id);
    EXPECT_EQ(20, outputs[1].id);
}

TEST_F(AtomicCyclicBufferTest, WriteMove_GivenFullBuffer_DoesNotMoveSourceData)
{
    AtomicCyclicBuffer<MoveTracker, 4> move_buf;

    // Fill it
    std::array<MoveTracker, 4> filler;
    for (int i = 0; i < 4; ++i) filler[i] = MoveTracker(i);
    move_buf.Write(std::span<MoveTracker>(filler));

    // Try write
    std::array<MoveTracker, 1> extra;
    extra[0] = MoveTracker(99);

    auto count = move_buf.Write(std::span<MoveTracker>(extra));

    EXPECT_EQ(0_size, count);
    EXPECT_TRUE(extra[0].IsValid());  // Should NOT be moved
}

// --------------------------------------------------------------------------------
// Read Tests
// --------------------------------------------------------------------------------

TEST_F(AtomicCyclicBufferTest, Read_GivenFullBuffer_ReadsRequestedAmount)
{
    std::array<int, kSize> data;
    data.fill(1);
    buffer_int.Write(std::span<const int>(data));

    std::array<int, 4> out;
    auto count = buffer_int.Read(std::span<int>(out));

    EXPECT_EQ(4_size, count);
    EXPECT_EQ(kSize - 4, buffer_int.Count());
}

TEST_F(AtomicCyclicBufferTest, Read_GivenEmptyBuffer_ReturnsZero)
{
    std::array<int, 4> out;
    auto count = buffer_int.Read(std::span<int>(out));
    EXPECT_EQ(0_size, count);
}

TEST_F(AtomicCyclicBufferTest, Read_GivenRequestLargerThanCount_ReadsAvailableOnly)
{
    std::array<int, 3> data = {1, 2, 3};
    buffer_int.Write(std::span<const int>(data));

    std::array<int, 10> out;
    auto count = buffer_int.Read(std::span<int>(out));

    EXPECT_EQ(3_size, count);
    EXPECT_TRUE(buffer_int.IsEmpty());
    EXPECT_EQ(1, out[0]);
    EXPECT_EQ(3, out[2]);
}

// --------------------------------------------------------------------------------
// State Transition & Edge Cases
// --------------------------------------------------------------------------------

TEST_F(AtomicCyclicBufferTest, Count_GivenAddAndRemoveSequences_ReturnsCorrectNumber)
{
    EXPECT_EQ(0_size, buffer_int.Count());

    int val = 1;
    buffer_int.Write(std::span<const int>(&val, 1));
    EXPECT_EQ(1_size, buffer_int.Count());

    buffer_int.Read(std::span<int>(&val, 1));
    EXPECT_EQ(0_size, buffer_int.Count());
}

TEST_F(AtomicCyclicBufferTest, IsFull_GivenOneSlotFree_ReturnsFalse)
{
    std::array<int, kSize - 1> data;
    data.fill(1);
    buffer_int.Write(std::span<const int>(data));

    EXPECT_FALSE(buffer_int.IsFull());

    int val = 1;
    buffer_int.Write(std::span<const int>(&val, 1));
    EXPECT_TRUE(buffer_int.IsFull());
}

TEST_F(AtomicCyclicBufferTest, IsEmpty_GivenOneItem_ReturnsFalse)
{
    int val = 1;
    buffer_int.Write(std::span<const int>(&val, 1));
    EXPECT_FALSE(buffer_int.IsEmpty());
}
