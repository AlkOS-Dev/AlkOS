#include <test_module/test.hpp>

#include <data_structures/tagged_pointer.hpp>

using namespace data_structures;

class TaggedPointerTest : public TestGroupBase
{
};

struct AlignedStruct1 {
    int value;
    AlignedStruct1(int v) : value(v) {}
};

struct AlignedStruct2 {
    long long a, b, c;

    AlignedStruct2(long long x, long long y, long long z) : a(x), b(y), c(z) {}
};

using TagPtr = TaggedPointer<AlignedStruct1, AlignedStruct2>;

TEST_F(TaggedPointerTest, BasicCreateAndIs)
{
    // Test Create with tag dispatch - uses universal reference binding
    auto ptr1 = TagPtr::Construct<AlignedStruct1>(42);
    R_ASSERT_TRUE(ptr1.IsValid());
    R_ASSERT_TRUE(ptr1.Is<AlignedStruct1>());
    R_ASSERT_FALSE(ptr1.Is<AlignedStruct2>());

    auto ptr2 = TagPtr::Construct<AlignedStruct2>(1LL, 2LL, 3LL);
    R_ASSERT_TRUE(ptr2.IsValid());
    R_ASSERT_FALSE(ptr2.Is<AlignedStruct1>());
    R_ASSERT_TRUE(ptr2.Is<AlignedStruct2>());
}

TEST_F(TaggedPointerTest, AsMethod)
{
    auto ptr1 = TagPtr::Construct<AlignedStruct1>(100);

    auto &s1 = ptr1.As<AlignedStruct1>();
    R_ASSERT_EQ(100, s1.value);

    auto ptr2 = TagPtr::Construct<AlignedStruct2>(10LL, 20LL, 30LL);

    auto &s2 = ptr2.As<AlignedStruct2>();
    R_ASSERT_EQ(10LL, s2.a);
    R_ASSERT_EQ(20LL, s2.b);
    R_ASSERT_EQ(30LL, s2.c);
}

TEST_F(TaggedPointerTest, MoveSemantics)
{
    auto ptr1 = TagPtr::Construct<AlignedStruct1>(123);
    R_ASSERT_TRUE(ptr1.IsValid());

    // Move construction
    auto ptr2 = std::move(ptr1);
    R_ASSERT_FALSE(ptr1.IsValid());  // moved-from object is invalid
    R_ASSERT_TRUE(ptr2.IsValid());
    R_ASSERT_EQ(123, ptr2.As<AlignedStruct1>().value);

    // Move assignment
    auto ptr3 = TagPtr::Construct<AlignedStruct2>(5LL, 10LL, 15LL);
    R_ASSERT_TRUE(ptr3.IsValid());

    ptr3 = std::move(ptr2);
    R_ASSERT_FALSE(ptr2.IsValid());  // moved-from object is invalid
    R_ASSERT_TRUE(ptr3.IsValid());
    R_ASSERT_TRUE(ptr3.Is<AlignedStruct1>());
    R_ASSERT_EQ(123, ptr3.As<AlignedStruct1>().value);
}
