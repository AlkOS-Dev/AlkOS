#include <extensions/type_traits.hpp>
#include <test_module/test.hpp>

using namespace std;

class TypeTraitsTest : public TestGroupBase
{
};

TEST_F(TypeTraitsTest, IsSame)
{
    EXPECT_TRUE((std::is_same_v<int, int>));
    EXPECT_TRUE((std::is_same_v<double, double>));
    EXPECT_TRUE((std::is_same_v<u64, uint64_t>));

    EXPECT_FALSE((std::is_same_v<float, double>));
    EXPECT_FALSE((std::is_same_v<float, int>));
    EXPECT_FALSE((std::is_same_v<u64, i32>));
}

TEST_F(TypeTraitsTest, RemoveReference)
{
    EXPECT_TRUE((std::is_same_v<int, std::remove_reference_t<int>>));
    EXPECT_TRUE((std::is_same_v<int, std::remove_reference_t<int &>>));
    EXPECT_TRUE((std::is_same_v<int, std::remove_reference_t<int &&>>));

    EXPECT_TRUE((std::is_same_v<double, std::remove_reference_t<double>>));
    EXPECT_TRUE((std::is_same_v<double, std::remove_reference_t<double &>>));
    EXPECT_TRUE((std::is_same_v<double, std::remove_reference_t<double &&>>));
}

TEST_F(TypeTraitsTest, Identity)
{
    const auto func = []<class T>(T a, std::type_identity_t<T> b) {
        return a + b;
    };

    EXPECT_TRUE((std::is_same_v<int, std::type_identity_t<int>>));
    EXPECT_TRUE((std::is_same_v<double, std::type_identity_t<double>>));
    EXPECT_TRUE((std::is_same_v<u64, std::type_identity_t<u64>>));

    EXPECT_EQ(4, func(2, 2.5));
}

TEST_F(TypeTraitsTest, AddPointer)
{
    EXPECT_TRUE((std::is_same_v<std::add_pointer_t<int>, int *>));
    EXPECT_TRUE((std::is_same_v<std::add_pointer_t<const int>, const int *>));
    EXPECT_TRUE((std::is_same_v<std::add_pointer_t<int &>, int *>));
    EXPECT_TRUE((std::is_same_v<std::add_pointer_t<const int &>, const int *>));
    EXPECT_TRUE((std::is_same_v<std::add_pointer_t<int &&>, int *>));
}

TEST_F(TypeTraitsTest, Conditional)
{
    EXPECT_TRUE((std::is_same_v<std::conditional_t<false, double, float>, float>));
    EXPECT_TRUE((std::is_same_v<std::conditional_t<true, double, float>, double>));
}

TEST_F(TypeTraitsTest, RemovePointer)
{
    EXPECT_TRUE((std::is_same_v<std::remove_pointer_t<int>, int>));
    EXPECT_TRUE((std::is_same_v<std::remove_pointer_t<int *>, int>));
    EXPECT_TRUE((std::is_same_v<std::remove_pointer_t<const int *>, const int>));
    EXPECT_TRUE((std::is_same_v<std::remove_pointer_t<int *const>, int>));
    EXPECT_TRUE((std::is_same_v<std::remove_pointer_t<int *const volatile>, int>));
    EXPECT_TRUE((std::is_same_v<std::remove_pointer_t<int *volatile>, int>));
}

TEST_F(TypeTraitsTest, RemoveExtent)
{
    EXPECT_TRUE((std::is_same_v<std::remove_extent_t<int>, int>));
    EXPECT_TRUE((std::is_same_v<std::remove_extent_t<int[]>, int>));
    EXPECT_TRUE((std::is_same_v<std::remove_extent_t<int[10]>, int>));

    int arr[]          = {1, 2, 3};
    const int arr1[]   = {1, 2, 3};
    const int arr2[10] = {1, 2, 3};

    EXPECT_TRUE((std::is_same_v<std::remove_extent_t<decltype(arr)>, int>));
    EXPECT_TRUE((std::is_same_v<std::remove_extent_t<decltype(arr1)>, const int>));
    EXPECT_TRUE((std::is_same_v<std::remove_extent_t<decltype(arr2)>, const int>));

    int dim2arr[2][2]{
        {1, 2},
        {3, 4}
    };

    EXPECT_TRUE((std::is_same_v<std::remove_extent_t<decltype(dim2arr)>, int[2]>));
    EXPECT_TRUE((
        std::is_same_v<std::remove_extent_t<std::remove_extent_t<decltype(dim2arr)>>, int>
    ));
}

TEST_F(TypeTraitsTest, IsConst)
{
    EXPECT_TRUE((std::is_const_v<const int>));
    EXPECT_FALSE((std::is_const_v<int>));
    EXPECT_FALSE((std::is_const_v<const int *>));
    EXPECT_FALSE((std::is_const_v<int *>));
    EXPECT_FALSE((std::is_const_v<const int &>));
    EXPECT_FALSE((std::is_const_v<int &>));
}

TEST_F(TypeTraitsTest, IsReference)
{
    EXPECT_TRUE((std::is_reference_v<int &>));
    EXPECT_TRUE((std::is_reference_v<int &&>));
    EXPECT_TRUE((std::is_reference_v<const int &>));
    EXPECT_FALSE((std::is_reference_v<const int>));
    EXPECT_FALSE((std::is_reference_v<int>));
}

static int f() { return 1; }

struct O {
    void operator()() {}
};

struct OO {
    static int foo() { return 2; }

    int fun() const &;
};

TEST_F(TypeTraitsTest, IsFunction)
{
    EXPECT_TRUE(std::is_function_v<decltype(f)>);
    EXPECT_TRUE(std::is_function_v<int(int)>);
    EXPECT_FALSE(std::is_function_v<int>);
    EXPECT_FALSE(std::is_function_v<decltype([] {
    })>);
    EXPECT_TRUE(std::is_function_v<O()>);
    EXPECT_FALSE(std::is_function_v<OO>);
    EXPECT_TRUE(std::is_function_v<decltype(OO::foo)>);
    EXPECT_FALSE(std::is_function_v<decltype(&OO::fun)>);
}

TEST_F(TypeTraitsTest, IsArray)
{
    EXPECT_FALSE((std::is_array_v<int>));
    EXPECT_TRUE((std::is_array_v<int[]>));
    EXPECT_TRUE((std::is_array_v<int[10]>));

    int arr[]          = {1, 2, 3};
    const int arr1[]   = {1, 2, 3};
    const int arr2[10] = {1, 2, 3};

    EXPECT_TRUE((std::is_array_v<decltype(arr)>));
    EXPECT_TRUE((std::is_array_v<decltype(arr1)>));
    EXPECT_TRUE((std::is_array_v<decltype(arr2)>));

    int dim2arr[2][2]{
        {1, 2},
        {3, 4}
    };

    EXPECT_TRUE((std::is_array_v<decltype(dim2arr)>));
    EXPECT_TRUE((std::is_array_v<std::remove_extent_t<decltype(dim2arr)>>));
}

TEST_F(TypeTraitsTest, Decay)
{
    EXPECT_TRUE((std::is_same_v<std::decay_t<int>, int>));
    EXPECT_FALSE((std::is_same_v<std::decay_t<int>, float>));
    EXPECT_TRUE((std::is_same_v<std::decay_t<int &>, int>));
    EXPECT_TRUE((std::is_same_v<std::decay_t<int &&>, int>));
    EXPECT_TRUE((std::is_same_v<std::decay_t<const int &>, int>));
    EXPECT_TRUE((std::is_same_v<std::decay_t<int[2]>, int *>));
    EXPECT_FALSE((std::is_same_v<std::decay_t<int[4][2]>, int *>));
    EXPECT_FALSE((std::is_same_v<std::decay_t<int[4][2]>, int **>));
    EXPECT_TRUE((std::is_same_v<std::decay_t<int[4][2]>, int (*)[2]>));
    EXPECT_TRUE((std::is_same_v<std::decay_t<int(int)>, int (*)(int)>));
    EXPECT_TRUE((std::is_same_v<std::decay_t<const char *const &>, const char *>));
}

TEST_F(TypeTraitsTest, IsLvalueReference)
{
    EXPECT_TRUE(std::is_lvalue_reference_v<int &>);
    EXPECT_TRUE(std::is_lvalue_reference_v<const int &>);
    EXPECT_FALSE(std::is_lvalue_reference_v<int &&>);
    EXPECT_FALSE(std::is_lvalue_reference_v<int>);
}

TEST_F(TypeTraitsTest, IsRvalueReference)
{
    EXPECT_FALSE(std::is_rvalue_reference_v<int &>);
    EXPECT_FALSE(std::is_rvalue_reference_v<const int &>);
    EXPECT_TRUE(std::is_rvalue_reference_v<int &&>);
    EXPECT_FALSE(std::is_rvalue_reference_v<int>);
}

TEST_F(TypeTraitsTest, IsVoid)
{
    EXPECT_TRUE(std::is_void_v<void>);
    EXPECT_TRUE(std::is_void_v<const void>);
    EXPECT_TRUE(std::is_void_v<const volatile void>);
    EXPECT_FALSE(std::is_void_v<int>);
    EXPECT_FALSE(std::is_void_v<const int>);
    EXPECT_FALSE(std::is_void_v<const volatile int>);
    EXPECT_FALSE(std::is_void_v<void *>);
    EXPECT_TRUE(std::is_void_v<volatile void>);
    EXPECT_FALSE(std::is_void_v<std::is_void<void>>);
}

TEST_F(TypeTraitsTest, IsNullPtr)
{
    EXPECT_TRUE(std::is_null_pointer_v<std::nullptr_t>);
    EXPECT_TRUE(std::is_null_pointer_v<const std::nullptr_t>);
    EXPECT_TRUE(std::is_null_pointer_v<const volatile std::nullptr_t>);
    EXPECT_TRUE(std::is_null_pointer_v<decltype(nullptr)>);
    EXPECT_FALSE(std::is_null_pointer_v<int>);
    EXPECT_FALSE(std::is_null_pointer_v<const int>);
    EXPECT_FALSE(std::is_null_pointer_v<void *>);
}

TEST_F(TypeTraitsTest, IsIntegral)
{
    EXPECT_TRUE(std::is_integral_v<int>);
    EXPECT_TRUE(std::is_integral_v<unsigned int>);
    EXPECT_TRUE(std::is_integral_v<short>);
    EXPECT_TRUE(std::is_integral_v<unsigned short>);
    EXPECT_TRUE(std::is_integral_v<long>);
    EXPECT_TRUE(std::is_integral_v<unsigned long>);
    EXPECT_TRUE(std::is_integral_v<long long>);
    EXPECT_TRUE(std::is_integral_v<unsigned long long>);
    EXPECT_TRUE(std::is_integral_v<char>);
    EXPECT_TRUE(std::is_integral_v<unsigned char>);
    EXPECT_TRUE(std::is_integral_v<signed char>);
    EXPECT_TRUE(std::is_integral_v<wchar_t>);
    EXPECT_TRUE(std::is_integral_v<char8_t>);
    EXPECT_TRUE(std::is_integral_v<char16_t>);
    EXPECT_TRUE(std::is_integral_v<char32_t>);

    EXPECT_FALSE(std::is_integral_v<float>);
    EXPECT_FALSE(std::is_integral_v<double>);
    EXPECT_FALSE(std::is_integral_v<long double>);
    EXPECT_FALSE(std::is_integral_v<void>);
    EXPECT_FALSE(std::is_integral_v<int *>);
    EXPECT_FALSE(std::is_integral_v<int &>);
    EXPECT_FALSE(std::is_integral_v<int[]>);
    EXPECT_FALSE(std::is_integral_v<int(int)>);
}

TEST_F(TypeTraitsTest, IsFloatingPoint)
{
    EXPECT_TRUE(std::is_floating_point_v<float>);
    EXPECT_TRUE(std::is_floating_point_v<double>);
    EXPECT_TRUE(std::is_floating_point_v<long double>);

    EXPECT_FALSE(std::is_floating_point_v<int>);
    EXPECT_FALSE(std::is_floating_point_v<unsigned int>);
    EXPECT_FALSE(std::is_floating_point_v<short>);
    EXPECT_FALSE(std::is_floating_point_v<unsigned short>);
    EXPECT_FALSE(std::is_floating_point_v<long>);
    EXPECT_FALSE(std::is_floating_point_v<unsigned long>);
    EXPECT_FALSE(std::is_floating_point_v<long long>);
    EXPECT_FALSE(std::is_floating_point_v<unsigned long long>);
    EXPECT_FALSE(std::is_floating_point_v<void>);
    EXPECT_FALSE(std::is_floating_point_v<float *>);
    EXPECT_FALSE(std::is_floating_point_v<float &>);
    EXPECT_FALSE(std::is_floating_point_v<float[]>);
    EXPECT_FALSE(std::is_floating_point_v<float(float)>);
}

union SampleUnion {
    int a;
    float b;
};

enum SampleEnum { Value1, Value2, Value3 };

TEST_F(TypeTraitsTest, IsUnion)
{
    EXPECT_TRUE(std::is_union_v<SampleUnion>);
    EXPECT_FALSE(std::is_union_v<int>);
    EXPECT_FALSE(std::is_union_v<float>);
    EXPECT_FALSE(std::is_union_v<SampleEnum>);
    EXPECT_FALSE(std::is_union_v<void>);
}

TEST_F(TypeTraitsTest, IsEnum)
{
    EXPECT_TRUE(std::is_enum_v<SampleEnum>);
    EXPECT_FALSE(std::is_enum_v<int>);
    EXPECT_FALSE(std::is_enum_v<float>);
    EXPECT_FALSE(std::is_enum_v<SampleUnion>);
    EXPECT_FALSE(std::is_enum_v<void>);
}

TEST_F(TypeTraitsTest, IsPointer)
{
    EXPECT_TRUE(std::is_pointer_v<int *>);
    EXPECT_TRUE(std::is_pointer_v<const int *>);
    EXPECT_TRUE(std::is_pointer_v<int *const>);
    EXPECT_TRUE(std::is_pointer_v<int *volatile>);
    EXPECT_TRUE(std::is_pointer_v<int *const volatile>);

    EXPECT_FALSE(std::is_pointer_v<int>);
    EXPECT_FALSE(std::is_pointer_v<const int>);
    EXPECT_FALSE(std::is_pointer_v<int &>);
    EXPECT_FALSE(std::is_pointer_v<int &&>);
    EXPECT_FALSE(std::is_pointer_v<int[]>);
    EXPECT_FALSE(std::is_pointer_v<int[10]>);
    EXPECT_FALSE(std::is_pointer_v<void>);
    EXPECT_FALSE(std::is_pointer_v<std::nullptr_t>);
}

TEST_F(TypeTraitsTest, IsClass)
{
    struct A {
    };
    class B
    {
    };
    union U {
        int a;
    };
    enum E { Value1, Value2 };
    enum class C {};

    auto lambda = []() {
    };

    EXPECT_TRUE((std::is_class_v<A>));
    EXPECT_TRUE((std::is_class_v<B>));

    EXPECT_TRUE((std::is_class_v<decltype(lambda)>));

    EXPECT_FALSE((std::is_class_v<U>));

    EXPECT_FALSE((std::is_class_v<E>));
    EXPECT_FALSE((std::is_class_v<C>));

    EXPECT_FALSE((std::is_class_v<int>));
    EXPECT_FALSE((std::is_class_v<double>));
    EXPECT_FALSE((std::is_class_v<int *>));
    EXPECT_FALSE((std::is_class_v<int &>));
}

struct TestClass {
    int member;
    void func() {}
};

TEST_F(TypeTraitsTest, IsFundamental)
{
    EXPECT_TRUE(std::is_fundamental_v<int>);
    EXPECT_TRUE(std::is_fundamental_v<float>);
    EXPECT_TRUE(std::is_fundamental_v<double>);
    EXPECT_TRUE(std::is_fundamental_v<char>);
    EXPECT_TRUE(std::is_fundamental_v<bool>);
    EXPECT_TRUE(std::is_fundamental_v<void>);
    EXPECT_TRUE(std::is_fundamental_v<std::nullptr_t>);

    EXPECT_FALSE(std::is_fundamental_v<int *>);
    EXPECT_FALSE(std::is_fundamental_v<int &>);
    EXPECT_FALSE(std::is_fundamental_v<int[]>);
    EXPECT_FALSE(std::is_fundamental_v<int(int)>);
    EXPECT_FALSE(std::is_fundamental_v<TestClass>);
}

TEST_F(TypeTraitsTest, IsMemberPointer)
{
    EXPECT_TRUE(std::is_member_pointer_v<int TestClass::*>);
    EXPECT_TRUE(std::is_member_pointer_v<void (TestClass::*)()>);

    EXPECT_FALSE(std::is_member_pointer_v<int *>);
    EXPECT_FALSE(std::is_member_pointer_v<int>);
    EXPECT_FALSE(std::is_member_pointer_v<void()>);
}

TEST_F(TypeTraitsTest, IsScalar)
{
    EXPECT_TRUE(std::is_scalar_v<int>);
    EXPECT_TRUE(std::is_scalar_v<float>);
    EXPECT_TRUE(std::is_scalar_v<double>);
    EXPECT_TRUE(std::is_scalar_v<char>);
    EXPECT_TRUE(std::is_scalar_v<bool>);
    EXPECT_TRUE(std::is_scalar_v<std::nullptr_t>);
    EXPECT_TRUE(std::is_scalar_v<int *>);
    EXPECT_TRUE(std::is_scalar_v<int TestClass::*>);
    EXPECT_TRUE(std::is_scalar_v<void (TestClass::*)()>);

    EXPECT_FALSE(std::is_scalar_v<void>);
    EXPECT_FALSE(std::is_scalar_v<int[]>);
    EXPECT_FALSE(std::is_scalar_v<int(int)>);
    EXPECT_FALSE(std::is_scalar_v<TestClass>);
}

TEST_F(TypeTraitsTest, IsObject)
{
    struct TestClass {
    };
    union TestUnion {
        int a;
        float b;
    };

    EXPECT_TRUE(std::is_object_v<int>);
    EXPECT_TRUE(std::is_object_v<float>);
    EXPECT_TRUE(std::is_object_v<TestClass>);
    EXPECT_TRUE(std::is_object_v<TestUnion>);
    EXPECT_TRUE(std::is_object_v<int *>);
    EXPECT_TRUE(std::is_object_v<int[10]>);

    EXPECT_FALSE(std::is_object_v<void>);
    EXPECT_FALSE(std::is_object_v<int &>);
    EXPECT_FALSE(std::is_object_v<int(int)>);
}

TEST_F(TypeTraitsTest, IsSigned)
{
    EXPECT_TRUE(std::is_signed_v<int>);
    EXPECT_TRUE(std::is_signed_v<short>);
    EXPECT_TRUE(std::is_signed_v<long>);
    EXPECT_TRUE(std::is_signed_v<long long>);
    EXPECT_TRUE(std::is_signed_v<float>);
    EXPECT_TRUE(std::is_signed_v<double>);
    EXPECT_TRUE(std::is_signed_v<long double>);

    EXPECT_FALSE(std::is_signed_v<unsigned int>);
    EXPECT_FALSE(std::is_signed_v<unsigned short>);
    EXPECT_FALSE(std::is_signed_v<unsigned long>);
    EXPECT_FALSE(std::is_signed_v<unsigned long long>);
    EXPECT_FALSE(std::is_signed_v<bool>);
    EXPECT_TRUE(std::is_signed_v<char>);
}

TEST_F(TypeTraitsTest, IsUnsigned)
{
    EXPECT_TRUE(std::is_unsigned_v<unsigned int>);
    EXPECT_TRUE(std::is_unsigned_v<unsigned short>);
    EXPECT_TRUE(std::is_unsigned_v<unsigned long>);
    EXPECT_TRUE(std::is_unsigned_v<unsigned long long>);
    EXPECT_TRUE(std::is_unsigned_v<bool>);
    EXPECT_TRUE(std::is_unsigned_v<unsigned char>);

    EXPECT_FALSE(std::is_unsigned_v<int>);
    EXPECT_FALSE(std::is_unsigned_v<short>);
    EXPECT_FALSE(std::is_unsigned_v<long>);
    EXPECT_FALSE(std::is_unsigned_v<long long>);
    EXPECT_FALSE(std::is_unsigned_v<float>);
    EXPECT_FALSE(std::is_unsigned_v<double>);
    EXPECT_FALSE(std::is_unsigned_v<long double>);
}

TEST_F(TypeTraitsTest, IsAbstract)
{
    struct AbstractBase {
        virtual void func() = 0;
    };
    struct Derived : AbstractBase {
        void func() override {}
    };

    EXPECT_TRUE(std::is_abstract_v<AbstractBase>);
    EXPECT_FALSE(std::is_abstract_v<Derived>);
}

TEST_F(TypeTraitsTest, IsPolymorphic)
{
    struct Base {
        virtual void func() {}
    };
    struct Derived : Base {
    };
    struct NonVirtual {
    };

    EXPECT_TRUE(std::is_polymorphic_v<Base>);
    EXPECT_TRUE(std::is_polymorphic_v<Derived>);
    EXPECT_FALSE(std::is_polymorphic_v<NonVirtual>);
}

TEST_F(TypeTraitsTest, IsFinal)
{
    struct Regular {
    };
    struct FinalClass final {
    };

    EXPECT_FALSE(std::is_final_v<Regular>);
    EXPECT_TRUE(std::is_final_v<FinalClass>);
}

TEST_F(TypeTraitsTest, HasVirtualDestructor)
{
    struct Base {
        virtual ~Base() {}
    };
    struct Derived : Base {
    };
    struct NonVirtualDestructor {
        ~NonVirtualDestructor() {}
    };

    EXPECT_TRUE(std::has_virtual_destructor_v<Base>);
    EXPECT_TRUE(std::has_virtual_destructor_v<Derived>);
    EXPECT_FALSE(std::has_virtual_destructor_v<NonVirtualDestructor>);
    EXPECT_FALSE(std::has_virtual_destructor_v<int>);
    EXPECT_FALSE(std::has_virtual_destructor_v<void>);
}

TEST_F(TypeTraitsTest, IsStandardLayout)
{
    struct StandardLayoutStruct {
        int x;
        double y;
    };
    struct EmptyStruct {
    };

    EXPECT_TRUE(std::is_standard_layout_v<int>);
    EXPECT_TRUE(std::is_standard_layout_v<double>);
    EXPECT_TRUE(std::is_standard_layout_v<StandardLayoutStruct>);
    EXPECT_TRUE(std::is_standard_layout_v<EmptyStruct>);
    EXPECT_TRUE(std::is_standard_layout_v<int[10]>);

    struct NonStandardLayout1 {
        virtual void foo() {}
    };
    struct Base {
        int x;
    };
    struct Derived : Base {
        int y;
    };
    struct NonStandardLayout2 {
        private:
        int x;

        public:
        double y;
    };

    EXPECT_FALSE(std::is_standard_layout_v<NonStandardLayout1>);
    EXPECT_FALSE(std::is_standard_layout_v<Derived>);
    EXPECT_FALSE(std::is_standard_layout_v<NonStandardLayout2>);
}

TEST_F(TypeTraitsTest, IsTriviallyCopyable)
{
    struct TriviallyCopyable {
        int x;
        double y;
    };

    EXPECT_TRUE(std::is_trivially_copyable_v<int>);
    EXPECT_TRUE(std::is_trivially_copyable_v<double>);
    EXPECT_TRUE(std::is_trivially_copyable_v<TriviallyCopyable>);
    EXPECT_TRUE(std::is_trivially_copyable_v<int[10]>);

    struct NonTriviallyCopyable1 {
        int *ptr;
        NonTriviallyCopyable1(const NonTriviallyCopyable1 &other) { ptr = new int(*other.ptr); }
        ~NonTriviallyCopyable1() { delete ptr; }
    };

    struct NonTriviallyCopyable2 {
        char *buffer;
        NonTriviallyCopyable2() : buffer(nullptr) {}
        NonTriviallyCopyable2(const NonTriviallyCopyable2 &other)
        {
            if (other.buffer) {
                size_t len = 0;
                while (other.buffer[len]) len++;
                buffer = new char[len + 1];
                for (size_t i = 0; i <= len; i++) buffer[i] = other.buffer[i];
            }
        }
        ~NonTriviallyCopyable2() { delete[] buffer; }
    };

    EXPECT_FALSE(std::is_trivially_copyable_v<NonTriviallyCopyable1>);
    EXPECT_FALSE(std::is_trivially_copyable_v<NonTriviallyCopyable2>);
}

TEST_F(TypeTraitsTest, IsTrivial)
{
    struct TrivialType {
        int x;
        double y;
    };

    EXPECT_TRUE(std::is_trivial_v<int>);
    EXPECT_TRUE(std::is_trivial_v<double>);
    EXPECT_TRUE(std::is_trivial_v<TrivialType>);
    EXPECT_TRUE(std::is_trivial_v<int[10]>);

    struct NonTrivialType1 {
        NonTrivialType1() {}
    };

    struct NonTrivialType2 {
        int *data;
        NonTrivialType2() : data(new int(0)) {}
        ~NonTrivialType2() { delete data; }
    };

    struct NonTrivialType3 {
        virtual void foo() {}
    };

    EXPECT_FALSE(std::is_trivial_v<NonTrivialType1>);
    EXPECT_FALSE(std::is_trivial_v<NonTrivialType2>);
    EXPECT_FALSE(std::is_trivial_v<NonTrivialType3>);
}

TEST_F(TypeTraitsTest, IsPOD)
{
    struct PODType {
        int x;
        double y;
    };

    EXPECT_TRUE(std::is_pod_v<int>);
    EXPECT_TRUE(std::is_pod_v<double>);
    EXPECT_TRUE(std::is_pod_v<PODType>);
    EXPECT_TRUE(std::is_pod_v<int[10]>);

    struct NonPODType1 {
        NonPODType1() {}
    };

    struct NonPODType2 {
        int *data;
        NonPODType2() : data(new int(0)) {}
        ~NonPODType2() { delete data; }
    };

    struct NonPODType3 {
        virtual void foo() {}
    };

    EXPECT_FALSE(std::is_pod_v<NonPODType1>);
    EXPECT_FALSE(std::is_pod_v<NonPODType2>);
    EXPECT_FALSE(std::is_pod_v<NonPODType3>);
}

TEST_F(TypeTraitsTest, IsLayoutCompatible)
{
    struct A {
        int x;
    };
    struct B {
        int x;
    };

    struct C {
        int x;
        double y;
    };
    struct D {
        int x;
        double y;
    };

    struct E {
        int x;
        double y;
    };
    struct F {
        int x;
        float y;
    };

    struct G {
        int x;
        char padding[4];
        double y;
    };
    struct H {
        int x;
        double y;
    };

    EXPECT_TRUE((std::is_layout_compatible_v<A, B>));
    EXPECT_TRUE((std::is_layout_compatible_v<C, D>));
    EXPECT_TRUE((std::is_layout_compatible_v<int, int>));

    EXPECT_FALSE((std::is_layout_compatible_v<E, F>));
    EXPECT_FALSE((std::is_layout_compatible_v<G, H>));
    EXPECT_FALSE((std::is_layout_compatible_v<int, double>));
}

TEST_F(TypeTraitsTest, PointerInterconvertibleBaseOf)
{
    struct Foo {
    };

    struct Bar {
    };

    class Baz : Foo, public Bar
    {
        int x;
    };

    class NonStdLayout : public Baz
    {
        int y;
    };

    ASSERT_TRUE((std::is_pointer_interconvertible_base_of_v<Bar, Baz>));
    ASSERT_TRUE((std::is_pointer_interconvertible_base_of_v<Foo, Baz>));
    ASSERT_FALSE((std::is_pointer_interconvertible_base_of_v<Baz, NonStdLayout>));
    ASSERT_TRUE((std::is_pointer_interconvertible_base_of_v<NonStdLayout, NonStdLayout>));
}

TEST_F(TypeTraitsTest, IsConstructible)
{
    struct DefaultConstructible {
        int x;
        DefaultConstructible() : x(0) {}
    };

    struct NonDefaultConstructible {
        int x;
        NonDefaultConstructible() = delete;
        explicit NonDefaultConstructible(int val) : x(val) {}
    };

    struct ConvertibleFromInt {
        int x;
        ConvertibleFromInt(int val) : x(val) {}
    };

    EXPECT_TRUE((std::is_constructible_v<int>));
    EXPECT_TRUE((std::is_constructible_v<DefaultConstructible>));
    EXPECT_FALSE((std::is_constructible_v<NonDefaultConstructible>));

    EXPECT_TRUE((std::is_constructible_v<int, int>));
    EXPECT_TRUE((std::is_constructible_v<double, int>));
    EXPECT_TRUE((std::is_constructible_v<NonDefaultConstructible, int>));
    EXPECT_TRUE((std::is_constructible_v<ConvertibleFromInt, int>));

    EXPECT_FALSE((std::is_constructible_v<int, void *>));
    EXPECT_FALSE((std::is_constructible_v<DefaultConstructible, int>));
    EXPECT_TRUE((std::is_constructible_v<NonDefaultConstructible, double>));
}

TEST_F(TypeTraitsTest, IsTriviallyConstructible)
{
    struct TrivialDefault {
        int x;
    };

    struct NonTrivialDefault {
        int x;
        NonTrivialDefault() { x = 10; }
    };

    struct TrivialCopy {
        int x;
    };

    struct NonTrivialCopy {
        int x;
        NonTrivialCopy(const NonTrivialCopy &other) { x = other.x; }
    };

    EXPECT_TRUE((std::is_trivially_constructible_v<int>));
    EXPECT_TRUE((std::is_trivially_constructible_v<TrivialDefault>));
    EXPECT_FALSE((std::is_trivially_constructible_v<NonTrivialDefault>));

    EXPECT_TRUE((std::is_trivially_constructible_v<int, const int &>));
    EXPECT_TRUE((std::is_trivially_constructible_v<TrivialCopy, const TrivialCopy &>));
    EXPECT_FALSE((std::is_trivially_constructible_v<NonTrivialCopy, const NonTrivialCopy &>));

    EXPECT_TRUE((std::is_trivially_constructible_v<double, int>));
    EXPECT_FALSE((std::is_trivially_constructible_v<int *, int>));
}

TEST_F(TypeTraitsTest, IsNothrowConstructible)
{
    struct NothrowDefault {
        NothrowDefault() noexcept = default;
    };

    struct ThrowingDefault {
        ThrowingDefault() {}
    };

    struct ExplicitThrowingDefault {
        ExplicitThrowingDefault() noexcept(false) {}
    };

    struct NothrowConversion {
        NothrowConversion(int) noexcept {}
    };

    struct ThrowingConversion {
        ThrowingConversion(int) {}
    };

    EXPECT_TRUE((std::is_nothrow_constructible_v<int>));
    EXPECT_TRUE((std::is_nothrow_constructible_v<NothrowDefault>));
    EXPECT_FALSE((std::is_nothrow_constructible_v<ThrowingDefault>));
    EXPECT_FALSE((std::is_nothrow_constructible_v<ExplicitThrowingDefault>));

    EXPECT_TRUE((std::is_nothrow_constructible_v<int, int>));
    EXPECT_TRUE((std::is_nothrow_constructible_v<NothrowConversion, int>));
    EXPECT_FALSE((std::is_nothrow_constructible_v<ThrowingConversion, int>));
}

TEST_F(TypeTraitsTest, IsAssignable)
{
    struct HasAssignmentOperator {
        HasAssignmentOperator &operator=(const HasAssignmentOperator &) { return *this; }
        HasAssignmentOperator &operator=(int) { return *this; }
    };

    struct DeletedAssignmentOperator {
        DeletedAssignmentOperator &operator=(const DeletedAssignmentOperator &) = delete;
    };

    struct ConstMember {
        const int x;
    };

    EXPECT_TRUE((std::is_assignable_v<int &, int>));
    EXPECT_TRUE((std::is_assignable_v<double &, int>));
    EXPECT_TRUE((std::is_assignable_v<HasAssignmentOperator &, HasAssignmentOperator>));
    EXPECT_TRUE((std::is_assignable_v<HasAssignmentOperator &, int>));

    EXPECT_FALSE((std::is_assignable_v<int &, void *>));
    EXPECT_FALSE((std::is_assignable_v<DeletedAssignmentOperator &, DeletedAssignmentOperator>));
    EXPECT_FALSE((std::is_assignable_v<ConstMember &, ConstMember>));
    EXPECT_FALSE((std::is_assignable_v<const int &, int>));

    EXPECT_FALSE((std::is_assignable_v<int, int>));
}

TEST_F(TypeTraitsTest, IsMemberFunctionPointer)
{
    class TestClass
    {
        public:
        void memberFunction() {}
        int memberFunctionWithReturn() { return 0; }
        void memberFunctionWithArgs(int, double) {}
        void constMemberFunction() const {}
        static void staticMemberFunction() {}

        int memberVariable;
    };

    EXPECT_TRUE((std::is_member_function_pointer_v<void (TestClass::*)()>));
    EXPECT_TRUE((std::is_member_function_pointer_v<int (TestClass::*)()>));
    EXPECT_TRUE((std::is_member_function_pointer_v<void (TestClass::*)(int, double)>));
    EXPECT_TRUE((std::is_member_function_pointer_v<void (TestClass::*)() const>));

    EXPECT_TRUE((std::is_member_function_pointer_v<decltype(&TestClass::memberFunction)>));
    EXPECT_TRUE((
        std::is_member_function_pointer_v<decltype(&TestClass::memberFunctionWithReturn)>
    ));
    EXPECT_TRUE((std::is_member_function_pointer_v<decltype(&TestClass::memberFunctionWithArgs)>));
    EXPECT_TRUE((std::is_member_function_pointer_v<decltype(&TestClass::constMemberFunction)>));

    EXPECT_FALSE((std::is_member_function_pointer_v<void (*)()>));
    EXPECT_FALSE((std::is_member_function_pointer_v<decltype(&TestClass::staticMemberFunction)>));
    EXPECT_FALSE((std::is_member_function_pointer_v<decltype(&TestClass::memberVariable)>));
    EXPECT_FALSE((std::is_member_function_pointer_v<int TestClass::*>));
    EXPECT_FALSE((std::is_member_function_pointer_v<int>));
    EXPECT_FALSE((std::is_member_function_pointer_v<TestClass>));
}

struct CustomType {
};
struct Empty {
};
struct ComplexType {
    int a;
    double b;
    char c;
};
struct IncompleteType;

TEST_F(TypeTraitsTest, IsCompound)
{
    EXPECT_TRUE((is_compound_v<CustomType>));
    EXPECT_FALSE((is_compound_v<int>));
    EXPECT_FALSE((is_compound_v<void>));
}

TEST_F(TypeTraitsTest, IsArithmetic)
{
    EXPECT_TRUE((is_arithmetic_v<int>));
    EXPECT_TRUE((is_arithmetic_v<double>));
    EXPECT_FALSE((is_arithmetic_v<CustomType>));
    EXPECT_FALSE((is_arithmetic_v<void>));
}

TEST_F(TypeTraitsTest, IsVolatile)
{
    EXPECT_TRUE((is_volatile_v<volatile int>));
    EXPECT_FALSE((is_volatile_v<int>));
}

TEST_F(TypeTraitsTest, IsAggregate)
{
    struct Aggregate {
        int a;
        double b;
    };
    struct NonAggregate {
        NonAggregate() {}
    };
    EXPECT_TRUE((is_aggregate_v<Aggregate>));
    EXPECT_FALSE((is_aggregate_v<NonAggregate>));
}

TEST_F(TypeTraitsTest, IsBoundedArray)
{
    EXPECT_TRUE((is_bounded_array_v<int[5]>));
    EXPECT_FALSE((is_bounded_array_v<int[]>));
}

TEST_F(TypeTraitsTest, IsUnboundedArray)
{
    EXPECT_TRUE((is_unbounded_array_v<int[]>));
    EXPECT_FALSE((is_unbounded_array_v<int[5]>));
}

TEST_F(TypeTraitsTest, Rank)
{
    EXPECT_EQ(0_size, (rank_v<int>));
    EXPECT_EQ(1_size, (rank_v<int[5]>));
    EXPECT_EQ(2_size, (rank_v<int[3][4]>));
    EXPECT_EQ(3_size, (rank_v<int[3][4][2]>));
}

TEST_F(TypeTraitsTest, Extent)
{
    EXPECT_EQ(0_size, (extent_v<int>));
    EXPECT_EQ(5_size, (extent_v<int[5]>));
    EXPECT_EQ(3_size, (extent_v<int[3][4]>));
    EXPECT_EQ(3_size, (extent_v<int[3][4][2]>));
}

TEST_F(TypeTraitsTest, AlignmentOf)
{
    EXPECT_EQ(alignof(int), (alignment_of_v<int>));
    EXPECT_EQ(alignof(double), (alignment_of_v<double>));
    EXPECT_EQ(alignof(ComplexType), (alignment_of_v<ComplexType>));
}

TEST_F(TypeTraitsTest, IsInvocable)
{
    struct Functor {
        void operator()() {}
    };
    struct ParamFunctor {
        void operator()(int) {}
    };
    EXPECT_TRUE((is_invocable_v<Functor>));
    EXPECT_TRUE((is_invocable_v<ParamFunctor, int>));
    EXPECT_FALSE((is_invocable_v<ParamFunctor>));
}

TEST_F(TypeTraitsTest, IsInvocableR)
{
    struct Functor {
        int operator()() { return 42; }
    };
    EXPECT_TRUE((is_invocable_r_v<int, Functor>));
    EXPECT_TRUE((is_invocable_r_v<long, Functor>));
    EXPECT_TRUE((is_invocable_r_v<void, Functor>));
    EXPECT_FALSE((is_invocable_r_v<Functor, Functor>));
}

TEST_F(TypeTraitsTest, IsNothrowInvocable)
{
    struct Functor {
        void operator()() noexcept {}
    };
    struct ThrowingFunctor {
        void operator()() {}
    };
    EXPECT_TRUE((is_nothrow_invocable_v<Functor>));
    EXPECT_FALSE((is_nothrow_invocable_v<ThrowingFunctor>));
}

TEST_F(TypeTraitsTest, IsNothrowInvocableR)
{
    struct Functor {
        int operator()() noexcept { return 42; }
    };
    struct ThrowingFunctor {
        int operator()() { return 42; }
    };
    EXPECT_TRUE((is_nothrow_invocable_r_v<int, Functor>));
    EXPECT_FALSE((is_nothrow_invocable_r_v<int, ThrowingFunctor>));
}

TEST_F(TypeTraitsTest, IsBaseOf)
{
    struct Base {
    };
    struct Derived : Base {
    };
    struct Unrelated {
    };
    EXPECT_TRUE((is_base_of_v<Base, Derived>));
    EXPECT_FALSE((is_base_of_v<Derived, Base>));
    EXPECT_FALSE((is_base_of_v<Base, Unrelated>));
}

TEST_F(TypeTraitsTest, RemoveConst)
{
    EXPECT_TRUE((std::is_same_v<std::remove_const_t<const int>, int>));
    EXPECT_TRUE((std::is_same_v<std::remove_const_t<const double>, double>));
    EXPECT_TRUE((std::is_same_v<std::remove_const_t<const u64>, u64>));

    EXPECT_TRUE((std::is_same_v<std::remove_const_t<int>, int>));

    EXPECT_TRUE((std::is_same_v<std::remove_const_t<volatile int>, volatile int>));
    EXPECT_TRUE((std::is_same_v<std::remove_const_t<const volatile int>, volatile int>));
}

TEST_F(TypeTraitsTest, RemoveVolatile)
{
    EXPECT_TRUE((std::is_same_v<std::remove_volatile_t<volatile int>, int>));
    EXPECT_TRUE((std::is_same_v<std::remove_volatile_t<volatile double>, double>));
    EXPECT_TRUE((std::is_same_v<std::remove_volatile_t<volatile u64>, u64>));

    EXPECT_TRUE((std::is_same_v<std::remove_volatile_t<int>, int>));

    EXPECT_TRUE((std::is_same_v<std::remove_volatile_t<const int>, const int>));
    EXPECT_TRUE((std::is_same_v<std::remove_volatile_t<const volatile int>, const int>));
}

TEST_F(TypeTraitsTest, RemoveCV)
{
    EXPECT_TRUE((std::is_same_v<std::remove_cv_t<const int>, int>));
    EXPECT_TRUE((std::is_same_v<std::remove_cv_t<volatile int>, int>));
    EXPECT_TRUE((std::is_same_v<std::remove_cv_t<const volatile int>, int>));
    EXPECT_TRUE((std::is_same_v<std::remove_cv_t<const volatile double>, double>));

    EXPECT_TRUE((std::is_same_v<std::remove_cv_t<int>, int>));
    EXPECT_TRUE((std::is_same_v<std::remove_cv_t<u64>, u64>));

    EXPECT_TRUE((std::is_same_v<std::remove_cv_t<const int *>, const int *>));
    EXPECT_TRUE((std::is_same_v<std::remove_cv_t<int *const>, int *>));
}

TEST_F(TypeTraitsTest, AddConst)
{
    EXPECT_TRUE((std::is_same_v<std::add_const_t<int>, const int>));
    EXPECT_TRUE((std::is_same_v<std::add_const_t<double>, const double>));
    EXPECT_TRUE((std::is_same_v<std::add_const_t<u64>, const u64>));

    EXPECT_TRUE((std::is_same_v<std::add_const_t<const int>, const int>));
    EXPECT_TRUE((std::is_same_v<std::add_const_t<volatile int>, const volatile int>));
}

TEST_F(TypeTraitsTest, AddVolatile)
{
    EXPECT_TRUE((std::is_same_v<std::add_volatile_t<int>, volatile int>));
    EXPECT_TRUE((std::is_same_v<std::add_volatile_t<double>, volatile double>));
    EXPECT_TRUE((std::is_same_v<std::add_volatile_t<u64>, volatile u64>));

    EXPECT_TRUE((std::is_same_v<std::add_volatile_t<volatile int>, volatile int>));
    EXPECT_TRUE((std::is_same_v<std::add_volatile_t<const int>, const volatile int>));
}

TEST_F(TypeTraitsTest, AddCV)
{
    EXPECT_TRUE((std::is_same_v<std::add_cv_t<int>, const volatile int>));
    EXPECT_TRUE((std::is_same_v<std::add_cv_t<double>, const volatile double>));

    EXPECT_TRUE((std::is_same_v<std::add_cv_t<const int>, const volatile int>));
    EXPECT_TRUE((std::is_same_v<std::add_cv_t<volatile int>, const volatile int>));
    EXPECT_TRUE((std::is_same_v<std::add_cv_t<const volatile int>, const volatile int>));
}

TEST_F(TypeTraitsTest, AddLValueReference)
{
    EXPECT_TRUE((std::is_same_v<std::add_lvalue_reference_t<int>, int &>));
    EXPECT_TRUE((std::is_same_v<std::add_lvalue_reference_t<const int>, const int &>));
    EXPECT_TRUE((std::is_same_v<std::add_lvalue_reference_t<int &>, int &>));
    EXPECT_TRUE((std::is_same_v<std::add_lvalue_reference_t<int &&>, int &>));
    EXPECT_TRUE((std::is_same_v<std::add_lvalue_reference_t<void>, void>));
}

TEST_F(TypeTraitsTest, AddRValueReference)
{
    EXPECT_TRUE((std::is_same_v<std::add_rvalue_reference_t<int>, int &&>));
    EXPECT_TRUE((std::is_same_v<std::add_rvalue_reference_t<const int>, const int &&>));
    EXPECT_TRUE((std::is_same_v<std::add_rvalue_reference_t<int &>, int &>));
    EXPECT_TRUE((std::is_same_v<std::add_rvalue_reference_t<int &&>, int &&>));
    EXPECT_TRUE((std::is_same_v<std::add_rvalue_reference_t<void>, void>));
}

TEST_F(TypeTraitsTest, MakeSigned)
{
    EXPECT_TRUE((std::is_same_v<std::make_signed_t<unsigned int>, int>));
    EXPECT_TRUE((std::is_same_v<std::make_signed_t<u64>, int64_t>));
    EXPECT_TRUE((std::is_same_v<std::make_signed_t<char>, signed char>));
    EXPECT_TRUE((std::is_same_v<std::make_signed_t<unsigned char>, signed char>));
    EXPECT_TRUE((std::is_same_v<std::make_signed_t<int>, int>));
}

TEST_F(TypeTraitsTest, MakeUnsigned)
{
    EXPECT_TRUE((std::is_same_v<std::make_unsigned_t<int>, unsigned int>));
    EXPECT_TRUE((std::is_same_v<std::make_unsigned_t<i64>, uint64_t>));
    EXPECT_TRUE((std::is_same_v<std::make_unsigned_t<char>, unsigned char>));
    EXPECT_TRUE((std::is_same_v<std::make_unsigned_t<signed char>, unsigned char>));
    EXPECT_TRUE((std::is_same_v<std::make_unsigned_t<unsigned int>, unsigned int>));
}

TEST_F(TypeTraitsTest, RemoveAllExtents)
{
    EXPECT_TRUE((std::is_same_v<std::remove_all_extents_t<int>, int>));
    EXPECT_TRUE((std::is_same_v<std::remove_all_extents_t<int[5]>, int>));
    EXPECT_TRUE((std::is_same_v<std::remove_all_extents_t<int[5][10]>, int>));
    EXPECT_TRUE((std::is_same_v<std::remove_all_extents_t<int[][10]>, int>));
    EXPECT_TRUE((std::is_same_v<std::remove_all_extents_t<const int[5]>, const int>));
}

TEST_F(TypeTraitsTest, AlignedStorage)
{
    EXPECT_TRUE((sizeof(std::aligned_storage_t<1>) >= 1));
    EXPECT_TRUE((sizeof(std::aligned_storage_t<5>) >= 5));
    EXPECT_TRUE((sizeof(std::aligned_storage_t<10, 8>) >= 10));

    EXPECT_TRUE((alignof(std::aligned_storage_t<1>) >= 1));
    EXPECT_TRUE((alignof(std::aligned_storage_t<5, 4>) >= 4));
    EXPECT_TRUE((alignof(std::aligned_storage_t<10, 16>) >= 16));
}

TEST_F(TypeTraitsTest, AlignedUnion)
{
    EXPECT_TRUE((sizeof(std::aligned_union_t<0, char>) >= sizeof(char)));
    EXPECT_TRUE((sizeof(std::aligned_union_t<0, int, char>) >= sizeof(int)));
    EXPECT_TRUE((sizeof(std::aligned_union_t<10, char>) >= 10));
    EXPECT_TRUE((sizeof(std::aligned_union_t<20, char, short, int>) >= 20));

    EXPECT_TRUE((alignof(std::aligned_union_t<0, char, int>) >= alignof(int)));
    EXPECT_TRUE((alignof(std::aligned_union_t<0, double, int>) >= alignof(double)));
}

TEST_F(TypeTraitsTest, RemoveCVRef)
{
    EXPECT_TRUE((std::is_same_v<std::remove_cvref_t<int>, int>));
    EXPECT_TRUE((std::is_same_v<std::remove_cvref_t<const int>, int>));
    EXPECT_TRUE((std::is_same_v<std::remove_cvref_t<volatile int>, int>));
    EXPECT_TRUE((std::is_same_v<std::remove_cvref_t<const volatile int>, int>));

    EXPECT_TRUE((std::is_same_v<std::remove_cvref_t<int &>, int>));
    EXPECT_TRUE((std::is_same_v<std::remove_cvref_t<const int &>, int>));
    EXPECT_TRUE((std::is_same_v<std::remove_cvref_t<volatile int &>, int>));
    EXPECT_TRUE((std::is_same_v<std::remove_cvref_t<const volatile int &>, int>));

    EXPECT_TRUE((std::is_same_v<std::remove_cvref_t<int &&>, int>));
    EXPECT_TRUE((std::is_same_v<std::remove_cvref_t<const int &&>, int>));
    EXPECT_TRUE((std::is_same_v<std::remove_cvref_t<volatile int &&>, int>));
    EXPECT_TRUE((std::is_same_v<std::remove_cvref_t<const volatile int &&>, int>));

    struct Test {
    };
    EXPECT_TRUE((std::is_same_v<std::remove_cvref_t<Test>, Test>));
    EXPECT_TRUE((std::is_same_v<std::remove_cvref_t<const Test &>, Test>));
    EXPECT_TRUE((std::is_same_v<std::remove_cvref_t<volatile Test &&>, Test>));
}

TEST_F(TypeTraitsTest, UnderlyingType)
{
    enum E1 : int { A, B, C };
    enum E2 : unsigned char { X, Y, Z };
    enum class E3 : long long { P, Q, R };
    enum class E4 { M, N, O };

    EXPECT_TRUE((std::is_same_v<std::underlying_type_t<E1>, int>));
    EXPECT_TRUE((std::is_same_v<std::underlying_type_t<E2>, unsigned char>));
    EXPECT_TRUE((std::is_same_v<std::underlying_type_t<E3>, long long>));
    EXPECT_TRUE((std::is_same_v<std::underlying_type_t<E4>, int>));
}

int func(double) { return 0; }
TEST_F(TypeTraitsTest, ResultOf)
{
    EXPECT_TRUE((std::is_same_v<std::result_of_t<decltype(func) &(double)>, int>));

    auto lambda1 = [](int x) -> double {
        return x * 1.0;
    };
    auto lambda2 = []() {
        return 'a';
    };
    EXPECT_TRUE((std::is_same_v<std::result_of_t<decltype(lambda1)(int)>, double>));
    EXPECT_TRUE((std::is_same_v<std::result_of_t<decltype(lambda2)()>, char>));

    struct S {
        bool foo(int) { return true; }
        int bar() const { return 0; }
    };
    EXPECT_TRUE((std::is_same_v<std::result_of_t<decltype (&S::foo)(S *, int)>, bool>));
    EXPECT_TRUE((std::is_same_v<std::result_of_t<decltype (&S::bar)(const S *)>, int>));
}

TEST_F(TypeTraitsTest, InvokeResult)
{
    EXPECT_TRUE((std::is_same_v<std::invoke_result_t<decltype(func), double>, int>));

    int captured = 10;
    auto lambda1 = [](int x) -> double {
        return x * 1.0;
    };
    auto lambda2 = [captured](int x) -> int {
        return x + captured;
    };
    EXPECT_TRUE((std::is_same_v<std::invoke_result_t<decltype(lambda1), int>, double>));
    EXPECT_TRUE((std::is_same_v<std::invoke_result_t<decltype(lambda2), int>, int>));

    struct S {
        void foo() {}
        int bar(double) const { return 0; }
        static float baz(char) { return 0.0f; }
    };
    EXPECT_TRUE((std::is_same_v<std::invoke_result_t<decltype(&S::foo), S *>, void>));
    EXPECT_TRUE((std::is_same_v<std::invoke_result_t<decltype(&S::bar), const S *, double>, int>));
    EXPECT_TRUE((std::is_same_v<std::invoke_result_t<decltype(&S::baz), char>, float>));
}

TEST_F(TypeTraitsTest, UnwrapReference)
{
    EXPECT_TRUE((std::is_same_v<std::unwrap_reference_t<int>, int>));
    EXPECT_TRUE((std::is_same_v<std::unwrap_reference_t<int &>, int &>));

    EXPECT_TRUE((std::is_same_v<std::unwrap_reference_t<std::reference_wrapper<int>>, int &>));
    EXPECT_TRUE((
        std::is_same_v<std::unwrap_reference_t<std::reference_wrapper<const int>>, const int &>
    ));

    struct Test {
    };
    EXPECT_TRUE((std::is_same_v<std::unwrap_reference_t<Test>, Test>));
    EXPECT_TRUE((std::is_same_v<std::unwrap_reference_t<std::reference_wrapper<Test>>, Test &>));
}

TEST_F(TypeTraitsTest, UnwrapRefDecay)
{
    EXPECT_TRUE((std::is_same_v<std::unwrap_ref_decay_t<int>, int>));
    EXPECT_TRUE((std::is_same_v<std::unwrap_ref_decay_t<int &>, int>));
    EXPECT_TRUE((std::is_same_v<std::unwrap_ref_decay_t<const int &>, int>));

    EXPECT_TRUE((std::is_same_v<std::unwrap_ref_decay_t<int[3]>, int *>));
    EXPECT_TRUE((std::is_same_v<std::unwrap_ref_decay_t<int[][3]>, int (*)[3]>));

    EXPECT_TRUE((std::is_same_v<std::unwrap_ref_decay_t<std::reference_wrapper<int>>, int &>));
    EXPECT_TRUE((
        std::is_same_v<std::unwrap_ref_decay_t<std::reference_wrapper<const int>>, const int &>
    ));
}

TEST_F(TypeTraitsTest, Conjunction)
{
    EXPECT_TRUE((std::conjunction_v<>));
    EXPECT_TRUE((std::conjunction_v<std::true_type>));
    EXPECT_FALSE((std::conjunction_v<std::false_type>));

    EXPECT_TRUE((std::conjunction_v<std::true_type, std::true_type>));
    EXPECT_FALSE((std::conjunction_v<std::true_type, std::false_type>));
    EXPECT_FALSE((std::conjunction_v<std::false_type, std::true_type>));
    EXPECT_FALSE((std::conjunction_v<std::false_type, std::false_type>));

    EXPECT_TRUE((std::conjunction_v<std::true_type, std::true_type, std::true_type>));
    EXPECT_FALSE((std::conjunction_v<std::true_type, std::false_type, std::true_type>));

    EXPECT_TRUE((std::conjunction_v<std::is_integral<int>, std::is_signed<int>>));
    EXPECT_FALSE((std::conjunction_v<std::is_integral<int>, std::is_unsigned<int>>));
}

TEST_F(TypeTraitsTest, Disjunction)
{
    EXPECT_FALSE((std::disjunction_v<>));
    EXPECT_TRUE((std::disjunction_v<std::true_type>));
    EXPECT_FALSE((std::disjunction_v<std::false_type>));

    EXPECT_TRUE((std::disjunction_v<std::true_type, std::true_type>));
    EXPECT_TRUE((std::disjunction_v<std::true_type, std::false_type>));
    EXPECT_TRUE((std::disjunction_v<std::false_type, std::true_type>));
    EXPECT_FALSE((std::disjunction_v<std::false_type, std::false_type>));

    EXPECT_TRUE((std::disjunction_v<std::false_type, std::true_type, std::false_type>));
    EXPECT_FALSE((std::disjunction_v<std::false_type, std::false_type, std::false_type>));

    EXPECT_TRUE((std::disjunction_v<std::is_integral<int>, std::is_floating_point<int>>));
    EXPECT_FALSE((std::disjunction_v<std::is_floating_point<int>, std::is_void<int>>));
}

TEST_F(TypeTraitsTest, Negation)
{
    EXPECT_TRUE((std::negation_v<std::false_type>));
    EXPECT_FALSE((std::negation_v<std::true_type>));
    EXPECT_TRUE((std::negation_v<std::is_void<int>>));
    EXPECT_FALSE((std::negation_v<std::is_integral<int>>));
    EXPECT_TRUE((std::negation_v<std::is_floating_point<int>>));
}

TEST_F(TypeTraitsTest, IsConstantEvaluated)
{
    constexpr bool compile_time = std::is_constant_evaluated();
    bool runtime                = std::is_constant_evaluated();

    EXPECT_TRUE(compile_time);
    EXPECT_FALSE(runtime);

    auto test_runtime = []() {
        return std::is_constant_evaluated();
    };
    EXPECT_FALSE(test_runtime());
}
