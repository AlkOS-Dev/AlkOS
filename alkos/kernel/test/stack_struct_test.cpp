#include <extensions/array.hpp>
#include <extensions/template_lib.hpp>
#include <test_module/test.hpp>
#include "test_structs.hpp"

using namespace std;
using namespace template_lib;

// Test fixture for StaticStack
class StaticStackTest : public TestGroupBase
{
};

// Test fixture for SingleTypeStaticStack
class SingleTypeStaticStackTest : public TestGroupBase
{
};

class StaticRegisteryTest : public TestGroupBase
{
};

// ------------------------------
// StaticStack Tests
// ------------------------------

// Basic functionality tests
TEST_F(StaticStackTest, EmptyStackSize)
{
    ArrayStaticStack<1024> stack;
    EXPECT_EQ(0_size, stack.Size());
}

TEST_F(StaticStackTest, PushPopInt)
{
    ArrayStaticStack<1024> stack;

    stack.Push(42);
    EXPECT_EQ(sizeof(int), stack.Size());

    int popped = stack.Pop<int>();
    EXPECT_EQ(42, popped);
    EXPECT_EQ(0_size, stack.Size());
}

TEST_F(StaticStackTest, PushPopMultipleInts)
{
    ArrayStaticStack<1024> stack;

    stack.Push(1);
    stack.Push(2);
    stack.Push(3);

    EXPECT_EQ(3 * sizeof(int), stack.Size());

    EXPECT_EQ(3, stack.Pop<int>());
    EXPECT_EQ(2, stack.Pop<int>());
    EXPECT_EQ(1, stack.Pop<int>());

    EXPECT_EQ(0u, stack.Size());
}

TEST_F(StaticStackTest, MoveOnlyType)
{
    ArrayStaticStack<1024> stack;

    MoveOnlyInt original(42);
    stack.Push(std::move(original));

    EXPECT_EQ(sizeof(MoveOnlyInt), stack.Size());
    EXPECT_EQ(0, original.getValue());

    MoveOnlyInt popped = stack.Pop<MoveOnlyInt>();
    EXPECT_EQ(42, popped.getValue());
    EXPECT_EQ(0u, stack.Size());
}

TEST_F(StaticStackTest, MixedTypes)
{
    ArrayStaticStack<1024> stack;

    stack.Push(42);
    stack.Push(3.14159);
    stack.Push(CustomString("hello"));

    EXPECT_EQ(sizeof(int) + sizeof(double) + sizeof(CustomString), stack.Size());

    CustomString s = stack.Pop<CustomString>();
    double d       = stack.Pop<double>();
    int i          = stack.Pop<int>();

    EXPECT_EQ("hello", s);
    EXPECT_EQ(3.14159, d);
    EXPECT_EQ(42, i);
}

FAIL_TEST_F(StaticStackTest, StackOverflow)
{
    ArrayStaticStack<16> small_stack;

    small_stack.Push(42);
    small_stack.Push(CustomString("Too large"));
}

FAIL_TEST_F(StaticStackTest, StackUnderflow)
{
    ArrayStaticStack<1024> stack;
    stack.Pop<int>();
}

// ------------------------------
// SingleTypeStaticStack Tests
// ------------------------------

TEST_F(SingleTypeStaticStackTest, EmptyStackSize)
{
    ArraySingleTypeStaticStack<int, 10> stack;
    EXPECT_EQ(0u, stack.Size());
}

TEST_F(SingleTypeStaticStackTest, PushPopInt)
{
    ArraySingleTypeStaticStack<int, 10> stack;

    stack.Push(42);
    EXPECT_EQ(sizeof(int), stack.Size());

    int popped = stack.Pop();
    EXPECT_EQ(42, popped);
    EXPECT_EQ(0u, stack.Size());
}

TEST_F(SingleTypeStaticStackTest, PushPopMultipleInts)
{
    ArraySingleTypeStaticStack<int, 10> stack;

    for (int i = 0; i < 10; ++i) {
        stack.Push(i);
    }

    EXPECT_EQ(10 * sizeof(int), stack.Size());

    for (int i = 9; i >= 0; --i) {
        EXPECT_EQ(i, stack.Pop());
    }

    EXPECT_EQ(0u, stack.Size());
}

TEST_F(SingleTypeStaticStackTest, CustomAlignment)
{
    ArraySingleTypeStaticStack<AlignedStruct, 8> stack;

    AlignedStruct as{1.0, 2.0};
    stack.Push(as);

    for (int i = 0; i < 7; ++i) {
        AlignedStruct temp;
        temp.x = static_cast<double>(i);
        temp.y = static_cast<double>(i * 2);
        stack.Push(temp);
    }

    EXPECT_EQ(8 * sizeof(AlignedStruct), stack.Size());

    AlignedStruct last = stack.Pop();
    EXPECT_EQ(6.0, last.x);
    EXPECT_EQ(12.0, last.y);

    for (int i = 0; i < 7; ++i) {
        stack.Pop();
    }

    EXPECT_EQ(0u, stack.Size());
}

FAIL_TEST_F(SingleTypeStaticStackTest, CapacityLimit)
{
    ArraySingleTypeStaticStack<int, 3> stack;

    stack.Push(1);
    stack.Push(2);
    stack.Push(3);

    stack.Push(4);
}

TEST_F(SingleTypeStaticStackTest, MoveSemantics)
{
    ArraySingleTypeStaticStack<MoveOnlyInt, 5> stack;

    stack.Push(MoveOnlyInt(42));
    stack.Push(MoveOnlyInt(43));

    EXPECT_EQ(2 * sizeof(MoveOnlyInt), stack.Size());

    MoveOnlyInt popped = stack.Pop();
    EXPECT_EQ(43, popped.getValue());

    popped = stack.Pop();
    EXPECT_EQ(42, popped.getValue());
}

TEST_F(StaticStackTest, VariedAlignmentTypes)
{
    ArrayStaticStack<1024, 32> stack;

    char c = 'A';
    stack.Push(c);

    short s = 42;
    stack.Push(s);

    int i = 123;
    stack.Push(i);

    double d = 3.14;
    stack.Push(d);

    struct alignas(16) Vector4 {
        float x, y, z, w;
    };

    Vector4 v{1.0f, 2.0f, 3.0f, 4.0f};
    stack.Push(v);  // 16-byte alignment

    Vector4 poppedV = stack.Pop<Vector4>();
    EXPECT_EQ(1.0f, poppedV.x);
    EXPECT_EQ(2.0f, poppedV.y);
    EXPECT_EQ(3.0f, poppedV.z);
    EXPECT_EQ(4.0f, poppedV.w);

    double poppedD = stack.Pop<double>();
    EXPECT_EQ(3.14, poppedD);

    int poppedI = stack.Pop<int>();
    EXPECT_EQ(123, poppedI);

    short poppedS = stack.Pop<short>();
    EXPECT_EQ(42, poppedS);

    char poppedC = stack.Pop<char>();
    EXPECT_EQ('A', poppedC);
}

TEST_F(StaticStackTest, CopyConstructor)
{
    ArrayStaticStack<1024> stack1;
    stack1.Push(1);
    stack1.Push(2);
    stack1.Push(3);

    ArrayStaticStack<1024> stack2 = stack1;

    EXPECT_EQ(stack1.Size(), stack2.Size());
    EXPECT_EQ(3 * sizeof(int), stack2.Size());

    EXPECT_EQ(3, stack2.Pop<int>());
    EXPECT_EQ(2, stack2.Pop<int>());
    EXPECT_EQ(1, stack2.Pop<int>());

    EXPECT_EQ(3 * sizeof(int), stack1.Size());
}

// ------------------------------
// StackRegisterTest
// ------------------------------

TEST_F(StaticRegisteryTest, FunctionalityTest)
{
    StackBasedVector<int, 3> reg{};

    reg.RegisterObject(1);
    reg.RegisterObject(2);
    reg.RegisterObject(3);

    int idx{};
    for (const auto& obj : reg) {
        EXPECT_EQ(obj, ++idx);
    }
}
