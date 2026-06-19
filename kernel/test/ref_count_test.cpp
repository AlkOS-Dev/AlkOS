// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#include <test_module/test.hpp>

#include <data_structures/ref_count.hpp>
#include "mem/heap.hpp"

using namespace data_structures;
using namespace Mem;

class RefCountTest : public TestGroupBase
{
};

namespace
{
class RefCountedObject : public RefCounted<RefCountedObject, true>
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
}  // namespace

TEST_F(RefCountTest, BasicRefCounted)
{
    bool destroyed = false;
    auto obj_res   = KNew<RefCountedObject>(42, &destroyed);
    R_ASSERT_TRUE(obj_res);
    auto *obj = *obj_res;
    obj->SetDeleter([](RefCountedObject *ptr) {
        KDelete(ptr);
    });

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
    R_ASSERT_TRUE(destroyed);
}

TEST_F(RefCountTest, RefPtrBasic)
{
    bool destroyed = false;

    {
        auto obj_res = KNew<RefCountedObject>(42, &destroyed);
        R_ASSERT_TRUE(obj_res);
        (*obj_res)->SetDeleter([](RefCountedObject *ptr) {
            KDelete(ptr);
        });
        RefPtr ptr1(*obj_res);
        R_ASSERT_TRUE(ptr1);
        R_ASSERT_EQ(42, ptr1->value);
        R_ASSERT_EQ(1u, ptr1->GetRefCount());
        R_ASSERT_FALSE(destroyed);

        {
            RefPtr<RefCountedObject> ptr2 = ptr1;
            R_ASSERT_EQ(2u, ptr1->GetRefCount());
            R_ASSERT_EQ(2u, ptr2->GetRefCount());
            R_ASSERT_FALSE(destroyed);
        }

        R_ASSERT_EQ(1u, ptr1->GetRefCount());
        R_ASSERT_FALSE(destroyed);
    }

    R_ASSERT_TRUE(destroyed);
}

TEST_F(RefCountTest, RefPtrMove)
{
    bool destroyed = false;

    {
        auto obj_res = KNew<RefCountedObject>(100, &destroyed);
        R_ASSERT_TRUE(obj_res);
        (*obj_res)->SetDeleter([](RefCountedObject *ptr) {
            KDelete(ptr);
        });
        RefPtr ptr1(*obj_res);
        R_ASSERT_EQ(1u, ptr1->GetRefCount());

        RefPtr<RefCountedObject> ptr2 = std::move(ptr1);
        R_ASSERT_FALSE(ptr1);
        R_ASSERT_TRUE(ptr2);
        R_ASSERT_EQ(1u, ptr2->GetRefCount());
        R_ASSERT_FALSE(destroyed);
    }

    R_ASSERT_TRUE(destroyed);
}

TEST_F(RefCountTest, RefPtrReset)
{
    bool destroyed = false;
    auto obj_res   = KNew<RefCountedObject>(42, &destroyed);
    R_ASSERT_TRUE(obj_res);
    (*obj_res)->SetDeleter([](RefCountedObject *ptr) {
        KDelete(ptr);
    });
    RefPtr ptr(*obj_res);

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
    auto obj_res   = KNew<RefCountedObject>(42, &destroyed);
    R_ASSERT_TRUE(obj_res);
    auto *obj = *obj_res;
    obj->SetDeleter([](RefCountedObject *ptr) {
        KDelete(ptr);
    });

    obj->AddRef();
    R_ASSERT_EQ(1u, obj->GetRefCount());

    {
        auto ptr = RefPtr(obj, false);
        R_ASSERT_EQ(1u, ptr->GetRefCount());
        R_ASSERT_FALSE(destroyed);
    }

    R_ASSERT_TRUE(destroyed);
}

TEST_F(RefCountTest, RefPtrAssignment)
{
    bool destroyed1 = false;
    bool destroyed2 = false;

    auto obj1_res = KNew<RefCountedObject>(1, &destroyed1);
    R_ASSERT_TRUE(obj1_res);
    (*obj1_res)->SetDeleter([](RefCountedObject *ptr) {
        KDelete(ptr);
    });
    RefPtr ptr1(*obj1_res);

    auto obj2_res = KNew<RefCountedObject>(2, &destroyed2);
    R_ASSERT_TRUE(obj2_res);
    (*obj2_res)->SetDeleter([](RefCountedObject *ptr) {
        KDelete(ptr);
    });
    RefPtr ptr2(*obj2_res);

    R_ASSERT_EQ(1u, ptr1->GetRefCount());
    R_ASSERT_EQ(1u, ptr2->GetRefCount());

    ptr1 = ptr2;

    R_ASSERT_TRUE(destroyed1);
    R_ASSERT_FALSE(destroyed2);
    R_ASSERT_EQ(2u, ptr2->GetRefCount());
    R_ASSERT_EQ(ptr1.Get(), ptr2.Get());
}

TEST_F(RefCountTest, RefPtrComparison)
{
    auto ptr1     = MakeRefCounted<RefCountedObject>(1, nullptr);
    auto ptr2     = ptr1;
    auto obj3_res = KNew<RefCountedObject>(2, nullptr);
    R_ASSERT_TRUE(obj3_res);
    (*obj3_res)->SetDeleter([](RefCountedObject *ptr) {
        KDelete(ptr);
    });
    RefPtr ptr3(*obj3_res);
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
    auto obj_res   = KNew<RefCountedObject>(42, &destroyed);
    R_ASSERT_TRUE(obj_res);
    (*obj_res)->SetDeleter([](RefCountedObject *ptr) {
        KDelete(ptr);
    });
    RefPtr ptr(*obj_res);

    R_ASSERT_EQ(1u, ptr->GetRefCount());

    RefCountedObject *raw = ptr.Detach();
    R_ASSERT_FALSE(ptr);
    R_ASSERT_TRUE(raw != nullptr);
    R_ASSERT_EQ(1u, raw->GetRefCount());
    R_ASSERT_FALSE(destroyed);

    raw->Release();
    R_ASSERT_TRUE(destroyed);
}
