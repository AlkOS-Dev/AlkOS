#include <extensions/atomic.hpp>
#include <test_module/test.hpp>
#include <todo.hpp>

TODO_THREADING
// TODO: Add multithreaded tests for atomic operations

TODO_LIBATOMIC
// TODO: Add tests for types of non lock-free sizes (i.e. 16+ bytes)

class AtomicTest : public TestGroupBase
{
    public:
    struct PaddedStruct {
        char a;
        int b;
    };

    static_assert(
        !std::has_unique_object_representations_v<PaddedStruct>, "PaddedStruct must have padding"
    );
};

// ------------------------------
// Compile time tests
// ------------------------------

namespace
{

template <typename T>
concept has_fetch_add = requires(T &t) { t.fetch_add(typename T::difference_type{1}); };

template <typename T>
concept has_fetch_and = requires(T &t) { t.fetch_and(typename T::value_type{1}); };

template <typename T>
concept has_fetch_or = requires(T &t) { t.fetch_or(typename T::value_type{1}); };

template <typename T>
concept has_fetch_xor = requires(T &t) { t.fetch_xor(typename T::value_type{1}); };

template <typename T>
concept has_compound_add = requires(T &t) { t += typename T::difference_type{1}; };

template <typename T>
concept has_compound_and = requires(T &t) { t &= typename T::value_type{1}; };

template <typename T>
concept has_increment = requires(T &t) { ++t; };

template <typename T>
concept has_store = requires(T &t) { t.store(typename T::value_type{}); };

template <typename T>
concept has_exchange = requires(T &t) { t.exchange(typename T::value_type{}); };

template <typename T>
concept has_address = requires(T &t) { t.address(); };

}  // namespace

// ------------------------------
// AtomicIntegral concept tests
// ------------------------------

static_assert(std::internal::AtomicIntegral<int>);
static_assert(std::internal::AtomicIntegral<char>);
static_assert(std::internal::AtomicIntegral<signed char>);
static_assert(std::internal::AtomicIntegral<unsigned char>);
static_assert(std::internal::AtomicIntegral<short>);
static_assert(std::internal::AtomicIntegral<unsigned short>);
static_assert(std::internal::AtomicIntegral<long>);
static_assert(std::internal::AtomicIntegral<unsigned long>);
static_assert(std::internal::AtomicIntegral<long long>);
static_assert(std::internal::AtomicIntegral<unsigned long long>);

// Should exclude bool and floating point types
static_assert(!std::internal::AtomicIntegral<bool>);
static_assert(!std::internal::AtomicIntegral<float>);
static_assert(!std::internal::AtomicIntegral<double>);
static_assert(!std::internal::AtomicIntegral<void *>);

// ------------------------------
// IsAtomicObject trait tests
// ------------------------------

static_assert(!std::internal::IsAtomicObject<int>);
static_assert(!std::internal::IsAtomicObject<float>);
static_assert(!std::internal::IsAtomicObject<double>);
static_assert(!std::internal::IsAtomicObject<void *>);
static_assert(!std::internal::IsAtomicObject<int *>);

static_assert(std::internal::IsAtomicObject<AtomicTest::PaddedStruct>);

// ------------------------------
// Specialization interface tests
// ------------------------------

static_assert(has_fetch_add<std::atomic<int>>);
static_assert(has_fetch_and<std::atomic<int>>);
static_assert(has_fetch_or<std::atomic<int>>);
static_assert(has_fetch_xor<std::atomic<int>>);
static_assert(has_compound_add<std::atomic<int>>);
static_assert(has_compound_and<std::atomic<int>>);
static_assert(has_increment<std::atomic<int>>);

static_assert(has_fetch_add<std::atomic<float>>);
static_assert(!has_fetch_and<std::atomic<float>>);
static_assert(!has_fetch_or<std::atomic<float>>);
static_assert(!has_compound_and<std::atomic<float>>);
static_assert(!has_increment<std::atomic<float>>);

static_assert(has_fetch_add<std::atomic<int *>>);
static_assert(!has_fetch_and<std::atomic<int *>>);
static_assert(has_increment<std::atomic<int *>>);

static_assert(!has_fetch_add<std::atomic<bool>>);
static_assert(!has_fetch_and<std::atomic<bool>>);
static_assert(!has_increment<std::atomic<bool>>);

// ------------------------------
// AtomicRef specialization interface tests
// ------------------------------

static_assert(has_fetch_add<std::atomic_ref<int>>);
static_assert(has_fetch_and<std::atomic_ref<int>>);
static_assert(has_fetch_or<std::atomic_ref<int>>);
static_assert(has_fetch_xor<std::atomic_ref<int>>);
static_assert(has_compound_add<std::atomic_ref<int>>);
static_assert(has_compound_and<std::atomic_ref<int>>);
static_assert(has_increment<std::atomic_ref<int>>);

static_assert(has_fetch_add<std::atomic_ref<float>>);
static_assert(!has_fetch_and<std::atomic_ref<float>>);
static_assert(!has_fetch_or<std::atomic_ref<float>>);
static_assert(!has_compound_and<std::atomic_ref<float>>);
static_assert(!has_increment<std::atomic_ref<float>>);

static_assert(has_fetch_add<std::atomic_ref<int *>>);
static_assert(!has_fetch_and<std::atomic_ref<int *>>);
static_assert(has_increment<std::atomic_ref<int *>>);

// ------------------------------
// IsAtomic concept tests
// ------------------------------

static_assert(std::internal::IsAtomic<std::atomic<int>>);
static_assert(!std::internal::IsAtomic<const std::atomic<int>>);
static_assert(std::internal::IsAtomic<volatile std::atomic<int>>);
static_assert(!std::internal::IsAtomic<const volatile std::atomic<int>>);
static_assert(std::internal::IsAtomic<const std::atomic<int>, true>);

// ------------------------------
// AtomicRef Function Interface
// ------------------------------

static_assert(has_store<std::atomic_ref<int>>);
static_assert(has_exchange<std::atomic_ref<int>>);
static_assert(has_compound_add<std::atomic_ref<int>>);

static_assert(!has_store<std::atomic_ref<const int>>);
static_assert(!has_exchange<std::atomic_ref<const int>>);
static_assert(!has_compound_add<std::atomic_ref<const int>>);

static_assert(requires(std::atomic_ref<int> &r) { r.load(); });
static_assert(requires(std::atomic_ref<const int> &r) { r.load(); });
static_assert(has_address<std::atomic_ref<int>>);
static_assert(has_address<std::atomic_ref<const int>>);

// ------------------------------
// Type member verification
// ------------------------------

static_assert(std::is_same_v<std::atomic<int>::value_type, int>);
static_assert(std::is_same_v<std::atomic<int>::difference_type, int>);

static_assert(std::is_same_v<std::atomic<float>::value_type, float>);
static_assert(std::is_same_v<std::atomic<float>::difference_type, float>);

static_assert(std::is_same_v<std::atomic<int *>::value_type, int *>);
static_assert(std::is_same_v<std::atomic<int *>::difference_type, ptrdiff_t>);

static_assert(std::is_same_v<std::atomic_ref<int>::value_type, int>);
static_assert(std::is_same_v<std::atomic_ref<int>::difference_type, int>);

static_assert(std::is_same_v<std::atomic_ref<float>::value_type, float>);
static_assert(std::is_same_v<std::atomic_ref<float>::difference_type, float>);

static_assert(std::is_same_v<std::atomic_ref<int *>::value_type, int *>);
static_assert(std::is_same_v<std::atomic_ref<int *>::difference_type, ptrdiff_t>);

// ------------------------------
// Runtime tests
// ------------------------------

TEST_F(AtomicTest, Atomic_BasicOperations)
{
    std::atomic<int> a{42};
    ASSERT_EQ(42, a.load());

    a.store(100);
    ASSERT_EQ(100, a.load());

    ASSERT_EQ(100, a.exchange(200));
    ASSERT_EQ(200, a.load());

    int expected = 200;
    ASSERT_TRUE(a.compare_exchange_weak(expected, 300));
    ASSERT_EQ(300, a.load());
}

TEST_F(AtomicTest, AtomicRef_BasicOperations)
{
    int value = 42;
    std::atomic_ref<int> ref{value};

    ASSERT_EQ(42, ref.load());
    ASSERT_EQ(&value, ref.address());

    ref.store(100);
    ASSERT_EQ(100, value);
    ASSERT_EQ(100, ref.load());

    ASSERT_EQ(100, ref.exchange(200));
    ASSERT_EQ(200, value);
}

TEST_F(AtomicTest, AtomicFlag_BasicOperations)
{
    std::atomic_flag flag = ATOMIC_FLAG_INIT;

    ASSERT_FALSE(flag.test());
    ASSERT_FALSE(flag.test_and_set());
    ASSERT_TRUE(flag.test());

    flag.clear();
    ASSERT_FALSE(flag.test());

    // Test constructor
    std::atomic_flag flag_true{true};
    ASSERT_TRUE(flag_true.test());
}

TEST_F(AtomicTest, Atomic_ArithmeticOperations)
{
    std::atomic<int> a{10};

    ASSERT_EQ(10, a.fetch_add(5));
    ASSERT_EQ(15, a.load());
    ASSERT_EQ(15, a.fetch_sub(3));
    ASSERT_EQ(12, a.load());

    ASSERT_EQ(20, a += 8);
    ASSERT_EQ(15, a -= 5);
    ASSERT_EQ(16, ++a);
    ASSERT_EQ(15, --a);
}

TEST_F(AtomicTest, Atomic_BitwiseOperations)
{
    std::atomic<int> a{0xFF};

    ASSERT_EQ(0xFF, a.fetch_and(0x0F));
    ASSERT_EQ(0x0F, a.load());
    ASSERT_EQ(0x0F, a.fetch_or(0xF0));
    ASSERT_EQ(0xFF, a.load());
    ASSERT_EQ(0xFF, a.fetch_xor(0x55));
    ASSERT_EQ(0xAA, a.load());
}

TEST_F(AtomicTest, Atomic_PointerArithmeticOperations)
{
    int values[5] = {1, 2, 3, 4, 5};
    std::atomic<int *> a{&values[2]};

    ASSERT_EQ(&values[2], a.fetch_add(1));
    ASSERT_EQ(&values[3], a.load());
    ASSERT_EQ(&values[3], a.fetch_sub(2));
    ASSERT_EQ(&values[1], a.load());

    ASSERT_EQ(&values[3], a += 2);
    ASSERT_EQ(&values[4], ++a);
    ASSERT_EQ(&values[3], --a);
}

TEST_F(AtomicTest, Atomic_CompareExchange_NonZeroPadding)
{
    union PaddedAccess {
        PaddedStruct s;
        unsigned char bytes[sizeof(PaddedStruct)];
    };

    PaddedAccess value{
        .s{.a = 'X', .b = 42}
    };
    PaddedAccess expected{
        .s{.a = 'X', .b = 42}
    };

    // Explicitly modify padding bytes
    size_t padding_size = sizeof(PaddedStruct) - (sizeof(char) + sizeof(int));
    for (size_t i = sizeof(char); i < padding_size; i++) {
        value.bytes[i]    = 0x7F;
        expected.bytes[i] = 0xFF;
    }

    std::atomic<PaddedStruct> atomic_struct{value.s};

    // This should succeed despite different padding because the value representation match
    PaddedStruct desired{.a = 'Y', .b = 100};

    ASSERT_TRUE(atomic_struct.compare_exchange_strong(expected.s, desired));

    // Verify the exchange worked
    PaddedAccess result{.s = atomic_struct.load()};
    ASSERT_EQ('Y', result.s.a);
    ASSERT_EQ(100, result.s.b);
}

TEST_F(AtomicTest, AtomicRef_CompareExchange_NonZeroPadding)
{
    union PaddedAccess {
        PaddedStruct s;
        unsigned char bytes[sizeof(PaddedStruct)];
    };

    // Create value with modified padding
    PaddedAccess value{
        .s{.a = 'X', .b = 42}
    };

    // Create expected with different padding
    PaddedAccess expected{
        .s{.a = 'X', .b = 42}
    };

    // Explicitly modify padding bytes differently
    size_t padding_size = sizeof(PaddedStruct) - (sizeof(char) + sizeof(int));
    for (size_t i = sizeof(char); i < padding_size; i++) {
        value.bytes[i]    = 0x7F;
        expected.bytes[i] = 0xFF;
    }

    // Create atomic_ref to the value
    alignas(std::atomic_ref<PaddedStruct>::required_alignment) PaddedStruct &aligned_value =
        value.s;
    std::atomic_ref<PaddedStruct> ref{aligned_value};

    // This should succeed despite different padding because the value representation match
    PaddedStruct desired{.a = 'Y', .b = 100};

    ASSERT_TRUE(ref.compare_exchange_strong(expected.s, desired));

    // Verify the exchange worked
    ASSERT_EQ('Y', value.s.a);
    ASSERT_EQ(100, value.s.b);
}
