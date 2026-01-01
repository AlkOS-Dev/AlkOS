#include <test_module/test.hpp>

#include <data_structures/ref_count.hpp>

using namespace data_structures;

class RefCountTest : public TestGroupBase
{
};

class RefCountedObject : public RefCountedBase<RefCountedObject>
{
    public:
    int value;
    bool *destroyed_flag;

    RefCountedObject(int v, bool *flag) : value(v), destroyed_flag(flag) {}
    ~RefCountedObject() override
    {
        if (destroyed_flag) {
            *destroyed_flag = true;
        }
    }
};

TEST_F(RefCountTest, BasicRefCounted)
{
    bool destroyed = false;
    auto *obj      = new RefCountedObject(42, &destroyed);

    R_ASSERT_FALSE(obj->HasRefs());
    R_ASSERT_EQ(0u, obj->GetRefCount());

    obj->AddRef();
    R_ASSERT_TRUE(obj->HasRefs());
    R_ASSERT_EQ(1u, obj->GetRefCount());

    obj->AddRef();
    R_ASSERT_EQ(2u, obj->GetRefCount());

    obj->Release();
    R_ASSERT_EQ(1u, obj->GetRefCount());
    R_ASSERT_FALSE(destroyed);

    obj->Release();
    R_ASSERT_TRUE(destroyed);  // Object should be deleted
}

TEST_F(RefCountTest, RefPtrBasic)
{
    bool destroyed = false;

    {
        RefPtr<RefCountedObject> ptr1(new RefCountedObject(42, &destroyed));
        R_ASSERT_TRUE(ptr1);
        R_ASSERT_EQ(42, ptr1->value);
        R_ASSERT_EQ(1u, ptr1->GetRefCount());
        R_ASSERT_FALSE(destroyed);

        {
            RefPtr<RefCountedObject> ptr2 = ptr1;  // Copy
            R_ASSERT_EQ(2u, ptr1->GetRefCount());
            R_ASSERT_EQ(2u, ptr2->GetRefCount());
            R_ASSERT_FALSE(destroyed);
        }

        // ptr2 destroyed, ref count should be 1
        R_ASSERT_EQ(1u, ptr1->GetRefCount());
        R_ASSERT_FALSE(destroyed);
    }

    // ptr1 destroyed, object should be deleted
    R_ASSERT_TRUE(destroyed);
}

TEST_F(RefCountTest, RefPtrMove)
{
    bool destroyed = false;

    {
        RefPtr<RefCountedObject> ptr1(new RefCountedObject(100, &destroyed));
        R_ASSERT_EQ(1u, ptr1->GetRefCount());

        RefPtr<RefCountedObject> ptr2 = std::move(ptr1);
        R_ASSERT_FALSE(ptr1);  // ptr1 should be null
        R_ASSERT_TRUE(ptr2);
        R_ASSERT_EQ(1u, ptr2->GetRefCount());  // Count should still be 1
        R_ASSERT_FALSE(destroyed);
    }

    R_ASSERT_TRUE(destroyed);
}

TEST_F(RefCountTest, RefPtrReset)
{
    bool destroyed = false;
    RefPtr<RefCountedObject> ptr(new RefCountedObject(42, &destroyed));

    R_ASSERT_TRUE(ptr);
    R_ASSERT_FALSE(destroyed);

    ptr.Reset();
    R_ASSERT_FALSE(ptr);
    R_ASSERT_TRUE(destroyed);
}

TEST_F(RefCountTest, MakeRefCounted)
{
    bool destroyed = false;

    {
        auto ptr = MakeRefCounted<RefCountedObject>(42, &destroyed);
        R_ASSERT_TRUE(ptr);
        R_ASSERT_EQ(42, ptr->value);
        R_ASSERT_EQ(1u, ptr->GetRefCount());
        R_ASSERT_FALSE(destroyed);
    }

    R_ASSERT_TRUE(destroyed);
}

TEST_F(RefCountTest, AdoptRef)
{
    bool destroyed = false;
    auto *obj      = new RefCountedObject(42, &destroyed);

    // Manually increment ref count
    obj->AddRef();
    R_ASSERT_EQ(1u, obj->GetRefCount());

    {
        // Adopt the existing reference (don't increment)
        auto ptr = AdoptRef(obj);
        R_ASSERT_EQ(1u, ptr->GetRefCount());  // Still 1, not 2
        R_ASSERT_FALSE(destroyed);
    }

    R_ASSERT_TRUE(destroyed);
}

TEST_F(RefCountTest, RefPtrAssignment)
{
    bool destroyed1 = false;
    bool destroyed2 = false;

    RefPtr<RefCountedObject> ptr1(new RefCountedObject(1, &destroyed1));
    RefPtr<RefCountedObject> ptr2(new RefCountedObject(2, &destroyed2));

    R_ASSERT_EQ(1u, ptr1->GetRefCount());
    R_ASSERT_EQ(1u, ptr2->GetRefCount());

    ptr1 = ptr2;  // ptr1 now points to object 2

    R_ASSERT_TRUE(destroyed1);   // Object 1 should be destroyed
    R_ASSERT_FALSE(destroyed2);  // Object 2 still alive
    R_ASSERT_EQ(2u, ptr2->GetRefCount());
    R_ASSERT_EQ(ptr1.Get(), ptr2.Get());
}

TEST_F(RefCountTest, RefPtrComparison)
{
    auto ptr1 = MakeRefCounted<RefCountedObject>(1, nullptr);
    auto ptr2 = ptr1;
    RefPtr<RefCountedObject> ptr3(new RefCountedObject(2, nullptr));
    RefPtr<RefCountedObject> ptr_null;

    R_ASSERT_TRUE(ptr1 == ptr2);
    R_ASSERT_FALSE(ptr1 != ptr2);
    R_ASSERT_TRUE(ptr1 != ptr3);
    R_ASSERT_FALSE(ptr1 == ptr3);

    R_ASSERT_TRUE(ptr_null == nullptr);
    R_ASSERT_FALSE(ptr_null != nullptr);
    R_ASSERT_FALSE(ptr1 == nullptr);
    R_ASSERT_TRUE(ptr1 != nullptr);
}

TEST_F(RefCountTest, RefPtrDetach)
{
    bool destroyed = false;
    RefPtr<RefCountedObject> ptr(new RefCountedObject(42, &destroyed));

    R_ASSERT_EQ(1u, ptr->GetRefCount());

    RefCountedObject *raw = ptr.Detach();
    R_ASSERT_FALSE(ptr);  // ptr should be null now
    R_ASSERT_TRUE(raw != nullptr);
    R_ASSERT_EQ(1u, raw->GetRefCount());
    R_ASSERT_FALSE(destroyed);

    // Manually release the detached pointer
    raw->Release();
    R_ASSERT_TRUE(destroyed);
}
