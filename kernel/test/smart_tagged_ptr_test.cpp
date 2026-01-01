#include <test_module/test.hpp>

#include <data_structures/ref_count.hpp>
#include <data_structures/tagged_pointer.hpp>

using namespace data_structures;

class RefCountedTaggedPtrTest : public TestGroupBase
{
};

class RefCountedObject : public RefCountedBase<RefCountedObject>
{
    public:
    int value;
    bool *destroyed_flag;

    RefCountedObject(int v, bool *flag) : value(v), destroyed_flag(flag) {}

    virtual ~RefCountedObject()
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

TEST_F(RefCountedTaggedPtrTest, AutoDetectRefCounted)
{
    using TestPtr = NonOwningTaggedPtr<RefCountedObject, TestStruct>;

    bool destroyed = false;
    auto *obj      = new RefCountedObject(42, &destroyed);

    // Initially ref count is 0
    R_ASSERT_EQ(0u, obj->GetRefCount());

    {
        auto ptr = TestPtr::Wrap(obj);
        R_ASSERT_TRUE(ptr);
        R_ASSERT_TRUE(ptr.Is<RefCountedObject>());
        R_ASSERT_EQ(42, ptr.As<RefCountedObject>().value);
        R_ASSERT_EQ(1u, obj->GetRefCount());
        R_ASSERT_FALSE(destroyed);
    }

    // ptr destroyed - should call Release() and delete object
    R_ASSERT_TRUE(destroyed);
}

TEST_F(RefCountedTaggedPtrTest, CopyIncrementsRef)
{
    using TestPtr = NonOwningTaggedPtr<RefCountedObject>;

    bool destroyed = false;
    auto *obj      = new RefCountedObject(100, &destroyed);

    {
        auto ptr1 = TestPtr::Wrap(obj);
        R_ASSERT_EQ(1u, obj->GetRefCount());

        {
            // Copy auto-increments ref count for RefCounted types
            auto ptr2 = ptr1;
            R_ASSERT_EQ(2u, obj->GetRefCount());
            R_ASSERT_FALSE(destroyed);
        }

        // ptr2 destroyed, ref count should drop to 1
        R_ASSERT_EQ(1u, obj->GetRefCount());
        R_ASSERT_FALSE(destroyed);
    }

    // ptr1 destroyed, object should be deleted
    R_ASSERT_TRUE(destroyed);
}

TEST_F(RefCountedTaggedPtrTest, MoveDoesNotChangeRef)
{
    using TestPtr = NonOwningTaggedPtr<RefCountedObject>;

    bool destroyed = false;
    auto *obj      = new RefCountedObject(200, &destroyed);

    {
        auto ptr1 = TestPtr::Wrap(obj);
        R_ASSERT_EQ(1u, obj->GetRefCount());

        // Move should not change ref count
        auto ptr2 = std::move(ptr1);
        R_ASSERT_FALSE(ptr1);
        R_ASSERT_TRUE(ptr2);
        R_ASSERT_EQ(1u, obj->GetRefCount());
        R_ASSERT_FALSE(destroyed);
    }

    R_ASSERT_TRUE(destroyed);
}

TEST_F(RefCountedTaggedPtrTest, MixedRefCountedAndNonRefCounted)
{
    using TestPtr = NonOwningTaggedPtr<RefCountedObject, TestStruct>;

    bool destroyed = false;
    auto *ref_obj  = new RefCountedObject(42, &destroyed);
    TestStruct regular_obj(99);

    {
        // RefCounted type - auto ref counting
        auto ptr1 = TestPtr::Wrap(ref_obj);
        R_ASSERT_TRUE(ptr1.Is<RefCountedObject>());
        R_ASSERT_FALSE(ptr1.Is<TestStruct>());
        R_ASSERT_EQ(1u, ref_obj->GetRefCount());

        // Non-RefCounted type - no ref counting
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

    auto *obj1 = new RefCountedObject(1, &destroyed1);
    auto *obj2 = new RefCountedObject(2, &destroyed2);

    {
        auto ptr1 = TestPtr::Wrap(obj1);
        auto ptr2 = TestPtr::Wrap(obj2);

        R_ASSERT_EQ(1u, obj1->GetRefCount());
        R_ASSERT_EQ(1u, obj2->GetRefCount());

        // Assignment auto-releases old value and increments new value
        ptr1 = ptr2;

        R_ASSERT_TRUE(destroyed1);   // obj1 should be destroyed
        R_ASSERT_FALSE(destroyed2);  // obj2 still alive
        R_ASSERT_EQ(2u, obj2->GetRefCount());
    }

    R_ASSERT_TRUE(destroyed2);
}

TEST_F(RefCountedTaggedPtrTest, NonOwnedCanCopy)
{
    using NonOwnedPtr = NonOwningTaggedPtr<TestStruct>;

    TestStruct obj(42);
    auto ptr1 = NonOwnedPtr::Wrap(&obj);

    // This should compile - copy constructor should be available
    auto ptr2 = ptr1;
    R_ASSERT_TRUE(ptr2);
    R_ASSERT_EQ(42, ptr2.As<TestStruct>().value);

    // This should compile - copy assignment should be available
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
