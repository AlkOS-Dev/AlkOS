#include <test_module/test.hpp>

// Global counters to track calls and order
static int g_base_func_calls    = 0;
static int g_derived_func_calls = 0;
static int g_destruction_step   = 0;
static int g_base_dtor_step     = 0;
static int g_derived_dtor_step  = 0;

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

    // Call via base pointer should route to Derived
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
    // Use placement new to manually control lifetime without relying on global delete
    // which might try to free invalid memory if we used stack.
    alignas(DestructDerived) char buffer[sizeof(DestructDerived)];

    DestructBase *bptr = new (buffer) DestructDerived();

    // Manually invoke destructor via base pointer.
    // Since ~DestructBase is virtual, this should dispatch to ~DestructDerived first,
    // then implicitly call ~DestructBase.
    bptr->~DestructBase();

    // Derived destructor should run first (step 1)
    R_ASSERT_EQ(1, g_derived_dtor_step);
    // Base destructor should run second (step 2)
    R_ASSERT_EQ(2, g_base_dtor_step);
}

// --------------------------------------------------------
// Test 3: Abstract Base Class (Pure Virtual)
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
// Test 4: Multiple Inheritance & Thunks
// --------------------------------------------------------
// This tests if vtable lookups work correctly when casting between multiple base classes.

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

    // Ensure pointer adjustment happened (ptrB should likely differ from ptrA/d in address)
    // We don't strictly assert the address diff as it's implementation dependent,
    // but we check the behavior.

    // Call via InterfaceA
    R_ASSERT_EQ(10, ptrA->FuncA());

    // Call via InterfaceB (should use adjusted 'this' pointer in implementation if thunks work)
    R_ASSERT_EQ(20, ptrB->FuncB());

    // Direct calls
    R_ASSERT_EQ(10, d.FuncA());
    R_ASSERT_EQ(20, d.FuncB());
}
