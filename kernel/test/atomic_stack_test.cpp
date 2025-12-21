#include <data_structures/atomic_stack.hpp>
#include <test_module/test.hpp>

class AtomicArrayStaticStackTest : public TestGroupBase
{
    protected:
    static constexpr size_t kStackSize = 256;
    AtomicArrayStaticStack<kStackSize> stack_;
};

class AtomicArraySingleTypeStaticStackTest : public TestGroupBase
{
    protected:
    static constexpr size_t kNumObjects = 16;
    AtomicArraySingleTypeStaticStack<int, kNumObjects> stack_;
};

// --------------------------------------------------------------------------------
// AtomicArrayStaticStack - Constructor & Basic State Tests
// --------------------------------------------------------------------------------

TEST_F(AtomicArrayStaticStackTest, Constructor_GivenDefault_InitializesEmptyState)
{
    EXPECT_EQ(0_size, stack_.Size());
    EXPECT_NOT_NULL(stack_.Data());
}

TEST_F(AtomicArrayStaticStackTest, Size_GivenEmptyStack_ReturnsZero)
{
    EXPECT_EQ(0_size, stack_.Size());
}

TEST_F(AtomicArrayStaticStackTest, PushCopy_GivenInt_PushesSuccessfully)
{
    int value   = 42;
    int &pushed = stack_.Push(value);

    EXPECT_EQ(42, pushed);
    EXPECT_EQ(sizeof(int), stack_.Size());
}

TEST_F(AtomicArrayStaticStackTest, PushCopy_GivenMultipleInts_PushesInOrder)
{
    int val1 = 10;
    int val2 = 20;
    int val3 = 30;

    stack_.Push(val1);
    stack_.Push(val2);
    stack_.Push(val3);

    EXPECT_EQ(sizeof(int) * 3, stack_.Size());
}

TEST_F(AtomicArrayStaticStackTest, PushCopy_GivenDifferentTypes_PushesCorrectly)
{
    int int_val     = 42;
    char char_val   = 'A';
    float float_val = 3.14f;

    stack_.Push(int_val);
    stack_.Push(char_val);
    stack_.Push(float_val);

    EXPECT_EQ(sizeof(int) + sizeof(char) + sizeof(float), stack_.Size());
}

struct MoveOnlyType {
    int value;

    MoveOnlyType() = default;
    explicit MoveOnlyType(int v) : value(v) {}

    MoveOnlyType(const MoveOnlyType &)            = delete;
    MoveOnlyType &operator=(const MoveOnlyType &) = delete;

    MoveOnlyType(MoveOnlyType &&other) noexcept : value(other.value) { other.value = -1; }

    MoveOnlyType &operator=(MoveOnlyType &&other) noexcept
    {
        if (this != &other) {
            value       = other.value;
            other.value = -1;
        }
        return *this;
    }
};

TEST_F(AtomicArrayStaticStackTest, PushMove_GivenMoveOnlyType_MovesIntoStack)
{
    MoveOnlyType obj(100);
    MoveOnlyType &pushed = stack_.Push(std::move(obj));

    EXPECT_EQ(100, pushed.value);
    EXPECT_EQ(-1, obj.value);
    EXPECT_EQ(sizeof(MoveOnlyType), stack_.Size());
}

struct EmplaceTestType {
    int a;
    int b;
    int c;

    EmplaceTestType(int x, int y, int z) : a(x), b(y), c(z) {}
};

TEST_F(AtomicArrayStaticStackTest, PushEmplace_GivenArgs_ConstructsInPlace)
{
    EmplaceTestType &pushed = stack_.PushEmplace<EmplaceTestType>(1, 2, 3);

    EXPECT_EQ(1, pushed.a);
    EXPECT_EQ(2, pushed.b);
    EXPECT_EQ(3, pushed.c);
    EXPECT_EQ(sizeof(EmplaceTestType), stack_.Size());
}

TEST_F(AtomicArrayStaticStackTest, Pop_GivenPushedInt_ReturnsCorrectValue)
{
    int original = 42;
    stack_.Push(original);

    int popped = stack_.Pop<int>();

    EXPECT_EQ(42, popped);
    EXPECT_EQ(0_size, stack_.Size());
}

TEST_F(AtomicArrayStaticStackTest, Pop_GivenMultiplePushes_PopsInReverseOrder)
{
    stack_.Push(10);
    stack_.Push(20);
    stack_.Push(30);

    EXPECT_EQ(30, stack_.Pop<int>());
    EXPECT_EQ(20, stack_.Pop<int>());
    EXPECT_EQ(10, stack_.Pop<int>());
    EXPECT_EQ(0_size, stack_.Size());
}

TEST_F(AtomicArrayStaticStackTest, Pop_GivenDifferentTypes_PopsCorrectly)
{
    int int_val     = 42;
    char char_val   = 'A';
    float float_val = 3.14f;

    stack_.Push(int_val);
    stack_.Push(char_val);
    stack_.Push(float_val);

    EXPECT_EQ(float_val, stack_.Pop<float>());
    EXPECT_EQ(char_val, stack_.Pop<char>());
    EXPECT_EQ(int_val, stack_.Pop<int>());
}

TEST_F(AtomicArrayStaticStackTest, Size_GivenPushAndPop_UpdatesCorrectly)
{
    EXPECT_EQ(0_size, stack_.Size());

    stack_.Push(1);
    EXPECT_EQ(sizeof(int), stack_.Size());

    stack_.Push(2);
    EXPECT_EQ(sizeof(int) * 2, stack_.Size());

    stack_.Pop<int>();
    EXPECT_EQ(sizeof(int), stack_.Size());

    stack_.Pop<int>();
    EXPECT_EQ(0_size, stack_.Size());
}

TEST_F(AtomicArraySingleTypeStaticStackTest, Constructor_GivenDefault_InitializesEmptyState)
{
    EXPECT_EQ(0_size, stack_.Size());
    EXPECT_EQ(0_size, stack_.SizeBytes());
    EXPECT_NOT_NULL(stack_.Data());
}

TEST_F(AtomicArraySingleTypeStaticStackTest, Size_GivenEmptyStack_ReturnsZero)
{
    EXPECT_EQ(0_size, stack_.Size());
    EXPECT_EQ(0_size, stack_.SizeBytes());
}

TEST_F(AtomicArraySingleTypeStaticStackTest, PushCopy_GivenInt_PushesSuccessfully)
{
    int value   = 42;
    int &pushed = stack_.Push(value);

    EXPECT_EQ(42, pushed);
    EXPECT_EQ(1_size, stack_.Size());
    EXPECT_EQ(sizeof(int), stack_.SizeBytes());
}

TEST_F(AtomicArraySingleTypeStaticStackTest, PushCopy_GivenMultipleInts_PushesInOrder)
{
    stack_.Push(10);
    stack_.Push(20);
    stack_.Push(30);

    EXPECT_EQ(3_size, stack_.Size());
    EXPECT_EQ(sizeof(int) * 3, stack_.SizeBytes());
}

TEST_F(AtomicArraySingleTypeStaticStackTest, PushMove_GivenInt_MovesIntoStack)
{
    int value   = 100;
    int &pushed = stack_.Push(std::move(value));

    EXPECT_EQ(100, pushed);
    EXPECT_EQ(1_size, stack_.Size());
}

struct EmplaceIntType {
    int value;

    explicit EmplaceIntType(int v) : value(v) {}
};

TEST_F(AtomicArraySingleTypeStaticStackTest, PushEmplace_GivenArgs_ConstructsInPlace)
{
    AtomicArraySingleTypeStaticStack<EmplaceIntType, 8> emplace_stack;
    EmplaceIntType &pushed = emplace_stack.PushEmplace(42);

    EXPECT_EQ(42, pushed.value);
    EXPECT_EQ(1_size, emplace_stack.Size());
}

TEST_F(AtomicArraySingleTypeStaticStackTest, Pop_GivenPushedInt_ReturnsCorrectValue)
{
    int original = 42;
    stack_.Push(original);

    int popped = stack_.Pop();

    EXPECT_EQ(42, popped);
    EXPECT_EQ(0_size, stack_.Size());
}

TEST_F(AtomicArraySingleTypeStaticStackTest, Pop_GivenMultiplePushes_PopsInReverseOrder)
{
    stack_.Push(10);
    stack_.Push(20);
    stack_.Push(30);

    EXPECT_EQ(30, stack_.Pop());
    EXPECT_EQ(20, stack_.Pop());
    EXPECT_EQ(10, stack_.Pop());
    EXPECT_EQ(0_size, stack_.Size());
}

TEST_F(AtomicArraySingleTypeStaticStackTest, Pop_GivenFullStack_PopsAllElements)
{
    for (int i = 0; i < static_cast<int>(kNumObjects); ++i) {
        stack_.Push(i);
    }

    EXPECT_EQ(kNumObjects, stack_.Size());

    for (int i = static_cast<int>(kNumObjects) - 1; i >= 0; --i) {
        EXPECT_EQ(i, stack_.Pop());
    }

    EXPECT_EQ(0_size, stack_.Size());
}

TEST_F(AtomicArraySingleTypeStaticStackTest, Size_GivenPushAndPop_UpdatesCorrectly)
{
    EXPECT_EQ(0_size, stack_.Size());

    stack_.Push(1);
    EXPECT_EQ(1_size, stack_.Size());
    EXPECT_EQ(sizeof(int), stack_.SizeBytes());

    stack_.Push(2);
    EXPECT_EQ(2_size, stack_.Size());
    EXPECT_EQ(sizeof(int) * 2, stack_.SizeBytes());

    stack_.Pop();
    EXPECT_EQ(1_size, stack_.Size());
    EXPECT_EQ(sizeof(int), stack_.SizeBytes());

    stack_.Pop();
    EXPECT_EQ(0_size, stack_.Size());
    EXPECT_EQ(0_size, stack_.SizeBytes());
}

TEST_F(
    AtomicArraySingleTypeStaticStackTest,
    PushPopSequence_GivenRepeatedOperations_MaintainsCorrectness
)
{
    for (int round = 0; round < 5; ++round) {
        for (int i = 0; i < 5; ++i) {
            stack_.Push(i + round * 10);
        }

        EXPECT_EQ(5_size, stack_.Size());

        for (int i = 4; i >= 0; --i) {
            EXPECT_EQ(i + round * 10, stack_.Pop());
        }

        EXPECT_EQ(0_size, stack_.Size());
    }
}
