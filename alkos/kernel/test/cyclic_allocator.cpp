#include <memory/cyclic_allocator.hpp>
#include <test_module/test.hpp>

class CyclicAllocatorTest : public TestGroupBase
{
};

class TestObject
{
    public:
    TestObject() noexcept = default;
    explicit TestObject(int value) : value_(value) {}
    ~TestObject() = default;

    int GetValue() const { return value_; }
    void SetValue(int value) { value_ = value; }

    private:
    int value_{};
};

TEST_F(CyclicAllocatorTest, BasicAllocFree)
{
    CyclicAllocator<TestObject, 8_size> allocator;

    EXPECT_EQ(allocator.GetFreeSlots(), 8_size);

    TestObject* obj = allocator.Allocate();
    EXPECT_EQ(allocator.GetFreeSlots(), 7_size);

    obj->SetValue(42);
    EXPECT_EQ(obj->GetValue(), 42);

    allocator.Free(obj);
    EXPECT_EQ(allocator.GetFreeSlots(), 8_size);
}

TEST_F(CyclicAllocatorTest, AllocateWithArgs)
{
    CyclicAllocator<TestObject, 4> allocator;

    TestObject* obj1 = allocator.Allocate(10);
    TestObject* obj2 = allocator.Allocate(20);

    EXPECT_EQ(obj1->GetValue(), 10);
    EXPECT_EQ(obj2->GetValue(), 20);

    allocator.Free(obj1);
    allocator.Free(obj2);
}

TEST_F(CyclicAllocatorTest, CyclicBehavior)
{
    CyclicAllocator<TestObject, 3> allocator;

    TestObject* obj1 = allocator.Allocate(1);
    TestObject* obj2 = allocator.Allocate(2);
    TestObject* obj3 = allocator.Allocate(3);

    EXPECT_EQ(allocator.GetFreeSlots(), 0_size);

    allocator.Free(obj2);
    EXPECT_EQ(allocator.GetFreeSlots(), 1_size);

    TestObject* obj4 = allocator.Allocate(4);
    EXPECT_EQ(allocator.GetFreeSlots(), 0_size);

    EXPECT_EQ(obj1->GetValue(), 1);
    EXPECT_EQ(obj3->GetValue(), 3);
    EXPECT_EQ(obj4->GetValue(), 4);

    allocator.Free(obj1);
    allocator.Free(obj3);
    allocator.Free(obj4);
}

TEST_F(CyclicAllocatorTest, FullAllocationCycle)
{
    CyclicAllocator<TestObject, 4> allocator;

    TestObject* objects[4];
    for (int i = 0; i < 4; ++i) {
        objects[i] = allocator.Allocate(i);
        EXPECT_EQ(objects[i]->GetValue(), i);
    }

    EXPECT_EQ(allocator.GetFreeSlots(), 0_size);

    for (int i = 3; i >= 0; --i) {
        allocator.Free(objects[i]);
        EXPECT_EQ(allocator.GetFreeSlots(), 4_size - i);
    }

    EXPECT_EQ(allocator.GetFreeSlots(), 4_size);

    for (int i = 0; i < 4; ++i) {
        objects[i] = allocator.Allocate(i + 10);
        EXPECT_EQ(objects[i]->GetValue(), i + 10);
    }

    for (int i = 0; i < 4; ++i) {
        allocator.Free(objects[i]);
    }
}

TEST_F(CyclicAllocatorTest, MixedAllocFree)
{
    CyclicAllocator<TestObject, 8> allocator;

    TestObject* obj1 = allocator.Allocate(1);
    TestObject* obj2 = allocator.Allocate(2);

    allocator.Free(obj1);

    TestObject* obj3 = allocator.Allocate(3);
    TestObject* obj4 = allocator.Allocate(4);

    allocator.Free(obj2);
    allocator.Free(obj4);

    TestObject* obj5 = allocator.Allocate(5);

    EXPECT_EQ(obj3->GetValue(), 3);
    EXPECT_EQ(obj5->GetValue(), 5);

    allocator.Free(obj3);
    allocator.Free(obj5);
}

TEST_F(CyclicAllocatorTest, ReuseMemory)
{
    CyclicAllocator<TestObject, 3> allocator;

    TestObject* addresses[3];

    for (int i = 0; i < 3; ++i) {
        addresses[i] = allocator.Allocate(i);
    }

    for (int i = 0; i < 3; ++i) {
        allocator.Free(addresses[i]);
    }

    TestObject* new_addresses[3];
    for (int i = 0; i < 3; ++i) {
        new_addresses[i] = allocator.Allocate(i + 10);
    }

    for (int i = 0; i < 3; ++i) {
        EXPECT_EQ(static_cast<void*>(addresses[i]), static_cast<void*>(new_addresses[i]));
        EXPECT_EQ(new_addresses[i]->GetValue(), i + 10);
    }

    for (int i = 0; i < 3; ++i) {
        allocator.Free(new_addresses[i]);
    }
}
