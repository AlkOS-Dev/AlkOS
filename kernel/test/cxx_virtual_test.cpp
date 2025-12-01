#include <test_module/test.hpp>

// Global counters
static int g_base_func_calls    = 0;
static int g_derived_func_calls = 0;
static int g_destruction_step   = 0;
static int g_base_dtor_step     = 0;
static int g_derived_dtor_step  = 0;

// Global state for Constructor/Destructor dispatch test
static int g_ctor_dispatch_val = 0;

class VirtualClassTest : public TestGroupBase
{
    public:
    void Setup_() override
    {
        g_base_func_calls    = 0;
        g_derived_func_calls = 0;
        g_destruction_step   = 0;
        g_base_dtor_step     = 0;
        g_derived_dtor_step  = 0;
        g_ctor_dispatch_val  = 0;
    }
};

// --------------------------------------------------------
// Test 1: Basic Virtual Function Dispatch
// --------------------------------------------------------

class Base
{
    public:
    virtual ~Base() = default;
    virtual void Func() { g_base_func_calls++; }
    virtual int GetValue() { return 10; }
};

class Derived : public Base
{
    public:
    void Func() override { g_derived_func_calls++; }
    int GetValue() override { return 20; }
};

TEST_F(VirtualClassTest, VirtualFunctionDispatch)
{
    Derived d;
    Base *bptr = &d;

    bptr->Func();

    R_ASSERT_EQ(0, g_base_func_calls);
    R_ASSERT_EQ(1, g_derived_func_calls);
    R_ASSERT_EQ(20, bptr->GetValue());
}

// --------------------------------------------------------
// Test 2: Virtual Destructor Order
// --------------------------------------------------------

class DestructBase
{
    public:
    virtual ~DestructBase() { g_base_dtor_step = ++g_destruction_step; }
};

class DestructDerived : public DestructBase
{
    public:
    ~DestructDerived() override { g_derived_dtor_step = ++g_destruction_step; }
};

TEST_F(VirtualClassTest, VirtualDestructorOrder)
{
    alignas(DestructDerived) char buffer[sizeof(DestructDerived)];
    DestructBase *bptr = new (buffer) DestructDerived();

    bptr->~DestructBase();

    R_ASSERT_EQ(1, g_derived_dtor_step);
    R_ASSERT_EQ(2, g_base_dtor_step);
}

// --------------------------------------------------------
// Test 3: Abstract Base Class
// --------------------------------------------------------

class AbstractInterface
{
    public:
    virtual ~AbstractInterface()        = default;
    virtual int Calculate(int a, int b) = 0;
};

class ConcreteImpl : public AbstractInterface
{
    public:
    int Calculate(int a, int b) override { return a + b; }
};

TEST_F(VirtualClassTest, AbstractClassUsage)
{
    ConcreteImpl impl;
    AbstractInterface *iptr = &impl;

    R_ASSERT_EQ(7, iptr->Calculate(3, 4));
}

// --------------------------------------------------------
// Test 4: Multiple Inheritance
// --------------------------------------------------------

class InterfaceA
{
    public:
    virtual ~InterfaceA() = default;
    virtual int FuncA() { return 1; }
};

class InterfaceB
{
    public:
    virtual ~InterfaceB() = default;
    virtual int FuncB() { return 2; }
};

class MultiDerived : public InterfaceA, public InterfaceB
{
    public:
    int FuncA() override { return 10; }
    int FuncB() override { return 20; }
};

TEST_F(VirtualClassTest, MultipleInheritance)
{
    MultiDerived d;
    InterfaceA *ptrA = &d;
    InterfaceB *ptrB = &d;

    // The address of ptrB should usually be offset from ptrA
    // due to object layout, though we don't assert the exact offset logic.

    R_ASSERT_EQ(10, ptrA->FuncA());
    R_ASSERT_EQ(20, ptrB->FuncB());
}

// --------------------------------------------------------
// Test 5: Virtual Dispatch During Construction/Destruction
// --------------------------------------------------------
// IMPORTANT: Calling a virtual function in a constructor MUST call the
// version defined in the class being constructed, NOT the derived class.

class CtorDispatchBase
{
    public:
    CtorDispatchBase()
    {
        // Should call CtorDispatchBase::Identify, NOT Derived
        Identify();
    }
    virtual ~CtorDispatchBase() = default;

    virtual void Identify() { g_ctor_dispatch_val += 1; }
};

class CtorDispatchDerived : public CtorDispatchBase
{
    public:
    CtorDispatchDerived()
    {
        // Should call CtorDispatchDerived::Identify
        Identify();
    }

    void Identify() override { g_ctor_dispatch_val += 10; }
};

TEST_F(VirtualClassTest, DispatchDuringConstruction)
{
    // 1. CtorDispatchBase ctor runs: calls Base::Identify (+1)
    // 2. CtorDispatchDerived ctor runs: calls Derived::Identify (+10)
    // Total should be 11. If vtable setup is broken, it might be 20 or crash.
    CtorDispatchDerived d;

    R_ASSERT_EQ(11, g_ctor_dispatch_val);
}

// --------------------------------------------------------
// Test 6: Virtual Inheritance (Diamond Problem)
// --------------------------------------------------------
// Checks correct offsets to shared base class.

class VirtualBase
{
    public:
    int data = 0;
    virtual void SetData(int v) { data = v; }
    virtual int GetData() { return data; }
    virtual ~VirtualBase() = default;
};

class Left : virtual public VirtualBase
{
    public:
    void SetData(int v) override { data = v + 1; }
};

class Right : virtual public VirtualBase
{
    // Inherits impl from VirtualBase
};

class DiamondBottom : public Left, public Right
{
    // Should imply there is only ONE 'data' integer shared across the object
};

TEST_F(VirtualClassTest, VirtualInheritanceDiamond)
{
    DiamondBottom bottom;
    VirtualBase *vptr = &bottom;
    Left *lptr        = &bottom;
    Right *rptr       = &bottom;

    // Verify there is only one instance of data
    bottom.data = 100;

    // Access via Left path (overridden setter adds 1)
    lptr->SetData(100);

    // Access via Right path (should see the change made by Left)
    // data should be 101 now.
    R_ASSERT_EQ(101, rptr->GetData());
    R_ASSERT_EQ(101, vptr->GetData());

    // Ensure pointers cast correctly back to the single base
    // Note: We cannot dynamic_cast, but static_cast works if hierarchy is known at compile time
    VirtualBase *vb_from_l = static_cast<VirtualBase *>(lptr);
    VirtualBase *vb_from_r = static_cast<VirtualBase *>(rptr);

    R_ASSERT_EQ(vb_from_l, vb_from_r);
}

// --------------------------------------------------------
// Test 7: Covariant Return Types
// --------------------------------------------------------
// Tests if the compiler generates thunks to adjust pointers on return.

class CovariantBase
{
    public:
    virtual ~CovariantBase() = default;
    virtual InterfaceA *GetInterface() { return nullptr; }
};

class CovariantDerived : public CovariantBase
{
    MultiDerived m_instance;  // MultiDerived inherits A and B

    public:
    // Returns MultiDerived*, which converts to InterfaceA*.
    // The compiler might need to adjust the pointer if InterfaceA isn't at offset 0 of
    // MultiDerived.
    InterfaceA *GetInterface() override { return &m_instance; }
};

TEST_F(VirtualClassTest, CovariantReturnTypes)
{
    CovariantDerived derived_obj;
    CovariantBase *base_ptr = &derived_obj;

    // Call via base pointer, but getting the derived implementation return value
    InterfaceA *iface = base_ptr->GetInterface();

    R_ASSERT_NOT_NULL(iface);
    R_ASSERT_EQ(10, iface->FuncA());
}
