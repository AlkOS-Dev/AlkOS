// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#include <test_module/test.hpp>

#include <data_structures/tagged_pointer.hpp>
#include "mem/heap.hpp"

using namespace data_structures;
using namespace Mem;

// ============================================================================
// TaggedPointer tests
// ============================================================================

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

using TagPtr = OwningTaggedPtr<AlignedStruct1, AlignedStruct2>;

TEST_F(TaggedPointerTest, BasicCreateAndIs)
{
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

    auto ptr2 = std::move(ptr1);
    R_ASSERT_FALSE(ptr1.IsValid());
    R_ASSERT_TRUE(ptr2.IsValid());
    R_ASSERT_EQ(123, ptr2.As<AlignedStruct1>().value);

    auto ptr3 = TagPtr::Construct<AlignedStruct2>(5LL, 10LL, 15LL);
    R_ASSERT_TRUE(ptr3.IsValid());

    ptr3 = std::move(ptr2);
    R_ASSERT_FALSE(ptr2.IsValid());
    R_ASSERT_TRUE(ptr3.IsValid());
    R_ASSERT_TRUE(ptr3.Is<AlignedStruct1>());
    R_ASSERT_EQ(123, ptr3.As<AlignedStruct1>().value);
}

// ============================================================================
// RefCountedTaggedPtr tests
// ============================================================================

class RefCountedTaggedPtrTest : public TestGroupBase
{
};

namespace
{
class RefCountedObject : public RefCounted<RefCountedObject, false>
{
    public:
    int value;
    bool *destroyed_flag;

    RefCountedObject(int v, bool *flag) : value(v), destroyed_flag(flag) {}

    ~RefCountedObject()
    {
        if (destroyed_flag) {
            *destroyed_flag = true;
        }
    }
};

struct TestStruct {
    int value;
    TestStruct(int v) : value(v) {}
};
}  // namespace

TEST_F(RefCountedTaggedPtrTest, AutoDetectRefCounted)
{
    using TestPtr = NonOwningTaggedPtr<RefCountedObject, TestStruct>;

    bool destroyed = false;
    auto obj       = RefCountedObject(42, &destroyed);

    R_ASSERT_EQ(0u, obj.GetRefCount());

    {
        auto ptr = TestPtr::Wrap(&obj);
        R_ASSERT_TRUE(ptr);
        R_ASSERT_TRUE(ptr.Is<RefCountedObject>());
        R_ASSERT_EQ(42, ptr.As<RefCountedObject>().value);
        R_ASSERT_EQ(1u, obj.GetRefCount());
        R_ASSERT_FALSE(destroyed);
    }

    R_ASSERT_TRUE(destroyed);
}

TEST_F(RefCountedTaggedPtrTest, CopyIncrementsRef)
{
    using TestPtr = NonOwningTaggedPtr<RefCountedObject>;

    bool destroyed = false;
    auto obj       = RefCountedObject(100, &destroyed);

    {
        auto ptr1 = TestPtr::Wrap(&obj);
        R_ASSERT_EQ(1u, obj.GetRefCount());

        {
            // Copy auto-increments ref count for RefCounted types
            auto ptr2 = ptr1;
            R_ASSERT_EQ(2u, obj.GetRefCount());
            R_ASSERT_FALSE(destroyed);
        }

        // ptr2 destroyed, ref count should drop to 1
        R_ASSERT_EQ(1u, obj.GetRefCount());
        R_ASSERT_FALSE(destroyed);
    }

    // ptr1 destroyed, object should be deleted
    R_ASSERT_TRUE(destroyed);
}

TEST_F(RefCountedTaggedPtrTest, MoveDoesNotChangeRef)
{
    using TestPtr = NonOwningTaggedPtr<RefCountedObject>;

    bool destroyed = false;
    auto obj       = RefCountedObject(200, &destroyed);

    {
        auto ptr1 = TestPtr::Wrap(&obj);
        R_ASSERT_EQ(1u, obj.GetRefCount());

        auto ptr2 = std::move(ptr1);
        R_ASSERT_FALSE(ptr1);
        R_ASSERT_TRUE(ptr2);
        R_ASSERT_EQ(1u, obj.GetRefCount());
        R_ASSERT_FALSE(destroyed);
    }

    R_ASSERT_TRUE(destroyed);
}

TEST_F(RefCountedTaggedPtrTest, MixedRefCountedAndNonRefCounted)
{
    using TestPtr = NonOwningTaggedPtr<RefCountedObject, TestStruct>;

    bool destroyed = false;
    auto obj       = RefCountedObject(42, &destroyed);
    TestStruct regular_obj(99);

    {
        auto ptr1 = TestPtr::Wrap(&obj);
        R_ASSERT_TRUE(ptr1.Is<RefCountedObject>());
        R_ASSERT_FALSE(ptr1.Is<TestStruct>());
        R_ASSERT_EQ(1u, obj.GetRefCount());

        auto ptr2 = TestPtr::Wrap(&regular_obj);
        R_ASSERT_FALSE(ptr2.Is<RefCountedObject>());
        R_ASSERT_TRUE(ptr2.Is<TestStruct>());
        R_ASSERT_EQ(99, ptr2.As<TestStruct>().value);

        R_ASSERT_FALSE(destroyed);
    }

    R_ASSERT_TRUE(destroyed);
}

TEST_F(RefCountedTaggedPtrTest, AssignmentHandlesRefCounting)
{
    using TestPtr = NonOwningTaggedPtr<RefCountedObject>;

    bool destroyed1 = false;
    bool destroyed2 = false;

    auto obj1 = RefCountedObject(1, &destroyed1);
    auto obj2 = RefCountedObject(2, &destroyed2);

    {
        auto ptr1 = TestPtr::Wrap(&obj1);
        auto ptr2 = TestPtr::Wrap(&obj2);

        R_ASSERT_EQ(1u, obj1.GetRefCount());
        R_ASSERT_EQ(1u, obj2.GetRefCount());

        ptr1 = ptr2;

        R_ASSERT_TRUE(destroyed1);   // obj1 should be destroyed
        R_ASSERT_FALSE(destroyed2);  // obj2 still alive
        R_ASSERT_EQ(2u, obj2.GetRefCount());
    }

    R_ASSERT_TRUE(destroyed2);
}

TEST_F(RefCountedTaggedPtrTest, NonOwnedCanCopy)
{
    using NonOwnedPtr = NonOwningTaggedPtr<TestStruct>;

    TestStruct obj(42);
    auto ptr1 = NonOwnedPtr::Wrap(&obj);

    auto ptr2 = ptr1;
    R_ASSERT_TRUE(ptr2);
    R_ASSERT_EQ(42, ptr2.As<TestStruct>().value);

    NonOwnedPtr ptr3;
    ptr3 = ptr1;
    R_ASSERT_TRUE(ptr3);
}

TEST_F(RefCountedTaggedPtrTest, MixedNonOwnedCanCopy)
{
    struct Obj1 {
        int a;
    };
    struct Obj2 {
        int b;
    };

    using MixedPtr = TaggedPointer<NonOwned<Obj1>, NonOwned<Obj2>>;

    Obj1 o1{1};
    Obj2 o2{2};

    auto ptr1 = MixedPtr::Wrap(&o1);
    auto ptr2 = MixedPtr::Wrap(&o2);

    // Both should be copyable
    auto ptr1_copy = ptr1;
    auto ptr2_copy = ptr2;

    R_ASSERT_TRUE(ptr1_copy.Is<Obj1>());
    R_ASSERT_TRUE(ptr2_copy.Is<Obj2>());
}
