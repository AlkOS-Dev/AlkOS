// alkos/kernel/test/expected_test.cpp

#include <extensions/defines.hpp>
#include <extensions/expected.hpp>
#include <extensions/utility.hpp>
#include <test_module/test.hpp>

//------------------------------------------------------------------------------
// Helper types for testing
//------------------------------------------------------------------------------

// A simple move-only type for testing move semantics.
struct MoveOnlyType {
    int value;
    explicit MoveOnlyType(int v) : value(v) {}
    MoveOnlyType(const MoveOnlyType &)            = delete;
    MoveOnlyType &operator=(const MoveOnlyType &) = delete;
    MoveOnlyType(MoveOnlyType &&other) noexcept : value(other.value) { other.value = -1; }
    MoveOnlyType &operator=(MoveOnlyType &&other) noexcept
    {
        value       = other.value;
        other.value = -1;
        return *this;
    }
};

// A type to track constructions and destructions, now fully featured.
struct LifecycleTracker {
    static int constructions;
    static int destructions;
    int value;

    explicit LifecycleTracker(int v = 0) noexcept : value(v) { constructions++; }
    LifecycleTracker(const LifecycleTracker &other) noexcept : value(other.value)
    {
        constructions++;
    }
    LifecycleTracker(LifecycleTracker &&other) noexcept : value(other.value)
    {
        constructions++;
        other.value = -1;
    }
    LifecycleTracker &operator=(const LifecycleTracker &other) noexcept
    {
        value = other.value;
        return *this;
    }
    LifecycleTracker &operator=(LifecycleTracker &&other) noexcept
    {
        value       = other.value;
        other.value = -1;
        return *this;
    }
    ~LifecycleTracker() { destructions++; }
    static void Reset()
    {
        constructions = 0;
        destructions  = 0;
    }
    void swap(LifecycleTracker &other) noexcept { std::swap(value, other.value); }
    bool operator==(const LifecycleTracker &other) const { return value == other.value; }
};
int LifecycleTracker::constructions = 0;
int LifecycleTracker::destructions  = 0;

void swap(LifecycleTracker &a, LifecycleTracker &b) noexcept { a.swap(b); }

// A type for testing in-place construction with multiple arguments.
struct MultiArgType {
    int x;
    double y;
    MultiArgType(int x, double y) : x(x), y(y) {}
    bool operator==(const MultiArgType &other) const { return x == other.x && y == other.y; }
};

// A type for testing construction with an initializer_list.
struct InitListType {
    int sum = 0;
    InitListType(std::initializer_list<int> il, int multiplier)
    {
        for (int val : il) {
            sum += val;
        }
        sum *= multiplier;
    }
};

class ExpectedTest : public TestGroupBase
{
};

#define SUCCEED() EXPECT_TRUE(true)

//------------------------------------------------------------------------------
// std::unexpected<E> Tests
//------------------------------------------------------------------------------

// Constructors

TEST_F(ExpectedTest, CopyConstructor_GivenUnexpected_CreatesIdenticalCopy)
{
    const std::unexpected<int> original(10);
    const std::unexpected<int> copy(original);
    EXPECT_EQ(10, copy.error());
}

TEST_F(ExpectedTest, MoveConstructor_GivenRValueUnexpected_MovesErrorAndLeavesSourceValid)
{
    std::unexpected<MoveOnlyType> original(MoveOnlyType(20));
    std::unexpected<MoveOnlyType> moved(std::move(original));
    EXPECT_EQ(20, moved.error().value);
    EXPECT_EQ(-1, original.error().value);  // Check source is in moved-from state
}

TEST_F(ExpectedTest, ConvertingConstructor_GivenCompatibleValue_InitializesError)
{
    const std::unexpected<int> unex(42.5);  // double is convertible to int
    EXPECT_EQ(42, unex.error());
}

TEST_F(ExpectedTest, ConvertingConstructor_GivenMoveOnlyError_MovesValueIntoError)
{
    MoveOnlyType mot(30);
    std::unexpected<MoveOnlyType> unex(std::move(mot));
    EXPECT_EQ(30, unex.error().value);
    EXPECT_EQ(-1, mot.value);
}

TEST_F(ExpectedTest, InPlaceConstructor_GivenArgs_ConstructsErrorInPlace)
{
    std::unexpected<MultiArgType> unex(std::in_place, 5, 3.14);
    EXPECT_EQ(5, unex.error().x);
    EXPECT_EQ(3.14, unex.error().y);
}

TEST_F(
    ExpectedTest,
    InPlaceInitializerListConstructor_GivenInitializerListAndArgs_ConstructsErrorInPlace
)
{
    std::unexpected<InitListType> unex(std::in_place, {1, 2, 3}, 10);
    EXPECT_EQ(60, unex.error().sum);
}

// Assignment

TEST_F(ExpectedTest, CopyAssignment_GivenUnexpected_CopiesError)
{
    const std::unexpected<int> source(50);
    std::unexpected<int> dest(0);
    dest = source;
    EXPECT_EQ(50, dest.error());
}

TEST_F(ExpectedTest, MoveAssignment_GivenRValueUnexpected_MovesError)
{
    std::unexpected<MoveOnlyType> source(MoveOnlyType(60));
    std::unexpected<MoveOnlyType> dest(MoveOnlyType(0));
    dest = std::move(source);
    EXPECT_EQ(60, dest.error().value);
    EXPECT_EQ(-1, source.error().value);
}

// Observers

TEST_F(ExpectedTest, ErrorConstLValue_GivenConstUnexpected_ReturnsConstLValueRefToError)
{
    const std::unexpected<int> unex(70);
    const int &err = unex.error();
    EXPECT_EQ(70, err);
    static_assert(std::is_same_v<decltype(unex.error()), const int &>);
}

TEST_F(ExpectedTest, ErrorLValue_GivenUnexpected_ReturnsLValueRefToError)
{
    std::unexpected<int> unex(80);
    unex.error() = 81;
    EXPECT_EQ(81, unex.error());
    static_assert(std::is_same_v<decltype(unex.error()), int &>);
}

TEST_F(ExpectedTest, ErrorConstRValue_GivenConstRValueUnexpected_ReturnsConstRValueRefToError)
{
    const std::unexpected<int> unex(90);
    const int &&err = std::move(unex).error();
    EXPECT_EQ(90, err);
    static_assert(std::is_same_v<decltype(std::move(unex).error()), const int &&>);
}

TEST_F(ExpectedTest, ErrorRValue_GivenRValueUnexpected_ReturnsRValueRefToError)
{
    std::unexpected<MoveOnlyType> unex(MoveOnlyType(100));
    MoveOnlyType moved_err = std::move(unex).error();
    EXPECT_EQ(100, moved_err.value);
    EXPECT_EQ(-1, unex.error().value);
    static_assert(std::is_same_v<decltype(std::move(unex).error()), MoveOnlyType &&>);
}

// Swap

TEST_F(ExpectedTest, Swap_GivenTwoUnexpecteds_SwapsTheirErrors)
{
    std::unexpected<LifecycleTracker> u1(LifecycleTracker(110));
    std::unexpected<LifecycleTracker> u2(LifecycleTracker(120));
    u1.swap(u2);
    EXPECT_EQ(120, u1.error().value);
    EXPECT_EQ(110, u2.error().value);

    swap(u1, u2);  // Test free function
    EXPECT_EQ(110, u1.error().value);
    EXPECT_EQ(120, u2.error().value);
}

// Comparison

TEST_F(ExpectedTest, OperatorEquals_GivenTwoEqualUnexpecteds_ReturnsTrue)
{
    const std::unexpected<int> u1(130);
    const std::unexpected<int> u2(130);
    EXPECT_TRUE(u1 == u2);
}

TEST_F(ExpectedTest, OperatorEquals_GivenTwoDifferentUnexpecteds_ReturnsFalse)
{
    const std::unexpected<int> u1(130);
    const std::unexpected<int> u2(140);
    EXPECT_FALSE(u1 == u2);
}

// Deduction Guide

TEST_F(ExpectedTest, DeductionGuide_GivenValue_DeducesCorrectUnexpectedType)
{
    std::unexpected unex(150);  // Deduces std::unexpected<int>
    static_assert(std::is_same_v<decltype(unex), std::unexpected<int>>);
    EXPECT_EQ(150, unex.error());
}

//------------------------------------------------------------------------------
// std::expected<T, E> Tests
//------------------------------------------------------------------------------

// Constructors

TEST_F(ExpectedTest, DefaultConstructor_GivenDefaultConstructibleValueType_ConstructsWithValue)
{
    std::expected<int, int> ex;
    EXPECT_TRUE(ex.has_value());
    EXPECT_EQ(0, *ex);
}

TEST_F(ExpectedTest, CopyConstructor_GivenValuedExpected_CopiesValue)
{
    const std::expected<LifecycleTracker, int> original(LifecycleTracker(10));
    const std::expected<LifecycleTracker, int> copy(original);
    EXPECT_TRUE(copy.has_value());
    EXPECT_EQ(10, copy->value);
}

TEST_F(ExpectedTest, CopyConstructor_GivenUnexpectedExpected_CopiesError)
{
    const std::expected<int, LifecycleTracker> original(std::unexpect, LifecycleTracker(20));
    const std::expected<int, LifecycleTracker> copy(original);
    EXPECT_FALSE(copy.has_value());
    EXPECT_EQ(20, copy.error().value);
}

TEST_F(ExpectedTest, MoveConstructor_GivenValuedExpected_MovesValue)
{
    std::expected<MoveOnlyType, int> original(MoveOnlyType(30));
    std::expected<MoveOnlyType, int> moved(std::move(original));
    EXPECT_TRUE(moved.has_value());
    EXPECT_EQ(30, moved->value);
    EXPECT_EQ(-1, original->value);
}

TEST_F(ExpectedTest, MoveConstructor_GivenUnexpectedExpected_MovesError)
{
    std::expected<int, MoveOnlyType> original(std::unexpect, MoveOnlyType(40));
    std::expected<int, MoveOnlyType> moved(std::move(original));
    EXPECT_FALSE(moved.has_value());
    EXPECT_EQ(40, moved.error().value);
    EXPECT_EQ(-1, original.error().value);
}

TEST_F(ExpectedTest, ConvertingCopyConstructor_GivenConvertibleValuedExpected_ConstructsWithValue)
{
    const std::expected<int, char> original(50);
    std::expected<double, int> converted(original);
    EXPECT_TRUE(converted.has_value());
    EXPECT_EQ(50.0, *converted);
}

TEST_F(
    ExpectedTest, ConvertingCopyConstructor_GivenConvertibleUnexpectedExpected_ConstructsWithError
)
{
    const std::expected<int, char> original(std::unexpect, 'a');
    std::expected<double, int> converted(original);
    EXPECT_FALSE(converted.has_value());
    EXPECT_EQ(97, converted.error());
}

TEST_F(
    ExpectedTest, ConvertingMoveConstructor_GivenConvertibleValuedExpected_ConstructsByMovingValue
)
{
    std::expected<MoveOnlyType, int> original(MoveOnlyType(60));
    std::expected<MoveOnlyType, double> converted(std::move(original));
    EXPECT_TRUE(converted.has_value());
    EXPECT_EQ(60, converted->value);
    EXPECT_EQ(-1, original->value);
}

TEST_F(
    ExpectedTest,
    ConvertingMoveConstructor_GivenConvertibleUnexpectedExpected_ConstructsByMovingError
)
{
    std::expected<int, MoveOnlyType> original(std::unexpect, MoveOnlyType(70));
    std::expected<double, MoveOnlyType> converted(std::move(original));
    EXPECT_FALSE(converted.has_value());
    EXPECT_EQ(70, converted.error().value);
    EXPECT_EQ(-1, original.error().value);
}

TEST_F(ExpectedTest, ValueConstructor_GivenValue_ConstructsWithValue)
{
    std::expected<int, double> ex = 80;
    EXPECT_TRUE(ex.has_value());
    EXPECT_EQ(80, *ex);
}

TEST_F(ExpectedTest, UnexpectedCopyConstructor_GivenConvertibleUnexpected_ConstructsWithError)
{
    const std::unexpected<char> unex('b');
    std::expected<int, int> ex(unex);
    EXPECT_FALSE(ex.has_value());
    EXPECT_EQ(98, ex.error());
}

TEST_F(ExpectedTest, UnexpectedMoveConstructor_GivenConvertibleUnexpected_ConstructsByMovingError)
{
    std::unexpected<MoveOnlyType> unex(MoveOnlyType(90));
    std::expected<int, MoveOnlyType> ex(std::move(unex));
    EXPECT_FALSE(ex.has_value());
    EXPECT_EQ(90, ex.error().value);
    EXPECT_EQ(-1, unex.error().value);
}

TEST_F(ExpectedTest, InPlaceValueConstructor_GivenArgs_ConstructsValueInPlace)
{
    std::expected<MultiArgType, int> ex(std::in_place, 100, 1.23);
    EXPECT_TRUE(ex.has_value());
    EXPECT_EQ(100, ex->x);
    EXPECT_EQ(1.23, ex->y);
}

TEST_F(ExpectedTest, InPlaceErrorConstructor_GivenArgs_ConstructsErrorInPlace)
{
    std::expected<int, MultiArgType> ex(std::unexpect, 200, 4.56);
    EXPECT_FALSE(ex.has_value());
    EXPECT_EQ(200, ex.error().x);
    EXPECT_EQ(4.56, ex.error().y);
}

// Destructor

TEST_F(ExpectedTest, Destructor_WhenHasValueWithNontrivialDestructor_CallsValueDestructor)
{
    LifecycleTracker::Reset();
    {
        std::expected<LifecycleTracker, int> ex(std::in_place, 1);
        EXPECT_EQ(1, LifecycleTracker::constructions);
    }
    EXPECT_EQ(1, LifecycleTracker::destructions);
}

TEST_F(ExpectedTest, Destructor_WhenHasErrorWithNontrivialDestructor_CallsErrorDestructor)
{
    LifecycleTracker::Reset();
    {
        std::expected<int, LifecycleTracker> ex(std::unexpect, 1);
        EXPECT_EQ(1, LifecycleTracker::constructions);
    }
    EXPECT_EQ(1, LifecycleTracker::destructions);
}

//  Assignment

TEST_F(ExpectedTest, CopyAssignment_FromValueToValue_AssignsValue)
{
    std::expected<LifecycleTracker, int> ex1(LifecycleTracker(10));
    std::expected<LifecycleTracker, int> ex2(LifecycleTracker(20));
    ex1 = ex2;
    EXPECT_TRUE(ex1.has_value());
    EXPECT_EQ(20, ex1->value);
}

TEST_F(ExpectedTest, CopyAssignment_FromErrorToValue_DestroysValueAndConstructsError)
{
    LifecycleTracker::Reset();
    std::expected<LifecycleTracker, int> ex1(LifecycleTracker(10));
    std::expected<LifecycleTracker, int> ex2(std::unexpect, 20);
    LifecycleTracker::Reset();

    ex1 = ex2;
    EXPECT_FALSE(ex1.has_value());
    EXPECT_EQ(20, ex1.error());
    EXPECT_EQ(1, LifecycleTracker::destructions);   // value destroyed
    EXPECT_EQ(0, LifecycleTracker::constructions);  // error is int, no construction tracked
}

TEST_F(ExpectedTest, CopyAssignment_FromValueToError_DestroysErrorAndConstructsValue)
{
    LifecycleTracker::Reset();
    std::expected<int, LifecycleTracker> ex1(std::unexpect, LifecycleTracker(10));
    std::expected<int, LifecycleTracker> ex2(20);
    LifecycleTracker::Reset();

    ex1 = ex2;
    EXPECT_TRUE(ex1.has_value());
    EXPECT_EQ(20, *ex1);
    EXPECT_EQ(1, LifecycleTracker::destructions);   // error destroyed
    EXPECT_EQ(0, LifecycleTracker::constructions);  // value is int, no construction tracked
}

TEST_F(ExpectedTest, CopyAssignment_FromErrorToError_AssignsError)
{
    std::expected<int, LifecycleTracker> ex1(std::unexpect, LifecycleTracker(10));
    std::expected<int, LifecycleTracker> ex2(std::unexpect, LifecycleTracker(20));
    ex1 = ex2;
    EXPECT_FALSE(ex1.has_value());
    EXPECT_EQ(20, ex1.error().value);
}

TEST_F(ExpectedTest, MoveAssignment_FromValueToValue_MovesValue)
{
    std::expected<MoveOnlyType, int> ex1(MoveOnlyType(10));
    std::expected<MoveOnlyType, int> ex2(MoveOnlyType(20));
    ex1 = std::move(ex2);
    EXPECT_TRUE(ex1.has_value());
    EXPECT_EQ(20, ex1->value);
    EXPECT_EQ(-1, ex2->value);
}

TEST_F(ExpectedTest, MoveAssignment_FromErrorToValue_DestroysValueAndMoveConstructsError)
{
    LifecycleTracker::Reset();
    std::expected<LifecycleTracker, MoveOnlyType> ex1(LifecycleTracker(10));
    std::expected<LifecycleTracker, MoveOnlyType> ex2(std::unexpect, MoveOnlyType(20));
    LifecycleTracker::Reset();

    ex1 = std::move(ex2);
    EXPECT_FALSE(ex1.has_value());
    EXPECT_EQ(20, ex1.error().value);
    EXPECT_EQ(1, LifecycleTracker::destructions);  // value destroyed
}

TEST_F(ExpectedTest, MoveAssignment_FromValueToError_DestroysErrorAndMoveConstructsValue)
{
    LifecycleTracker::Reset();
    std::expected<MoveOnlyType, LifecycleTracker> ex1(std::unexpect, LifecycleTracker(10));
    std::expected<MoveOnlyType, LifecycleTracker> ex2(MoveOnlyType(20));
    LifecycleTracker::Reset();

    ex1 = std::move(ex2);
    EXPECT_TRUE(ex1.has_value());
    EXPECT_EQ(20, ex1->value);
    EXPECT_EQ(1, LifecycleTracker::destructions);  // error destroyed
}

TEST_F(ExpectedTest, MoveAssignment_FromErrorToError_MovesError)
{
    std::expected<int, MoveOnlyType> ex1(std::unexpect, MoveOnlyType(10));
    std::expected<int, MoveOnlyType> ex2(std::unexpect, MoveOnlyType(20));
    ex1 = std::move(ex2);
    EXPECT_FALSE(ex1.has_value());
    EXPECT_EQ(20, ex1.error().value);
    EXPECT_EQ(-1, ex2.error().value);
}

TEST_F(ExpectedTest, ValueAssignment_ToValuedExpected_AssignsValue)
{
    std::expected<LifecycleTracker, int> ex(LifecycleTracker(10));
    ex = LifecycleTracker(100);
    EXPECT_TRUE(ex.has_value());
    EXPECT_EQ(100, ex->value);
}

TEST_F(ExpectedTest, ValueAssignment_ToUnexpectedExpected_DestroysErrorAndConstructsValue)
{
    LifecycleTracker::Reset();
    std::expected<LifecycleTracker, LifecycleTracker> ex(std::unexpect, 10);
    LifecycleTracker::Reset();
    ex = LifecycleTracker(100);
    EXPECT_TRUE(ex.has_value());
    EXPECT_EQ(100, ex->value);
    // 1 for temporary `LifecycleTracker(100)`
    // 1 for move-constructing into `ex`'s value storage
    EXPECT_EQ(2, LifecycleTracker::constructions);
    // 1 for destroying old error
    // 1 for destroying temporary
    EXPECT_EQ(2, LifecycleTracker::destructions);
}

TEST_F(ExpectedTest, UnexpectedAssignment_ToValuedExpected_DestroysValueAndConstructsError)
{
    LifecycleTracker::Reset();
    std::expected<LifecycleTracker, LifecycleTracker> ex(std::in_place, 10);
    LifecycleTracker::Reset();
    ex = std::unexpected(LifecycleTracker(100));
    EXPECT_FALSE(ex.has_value());
    EXPECT_EQ(100, ex.error().value);

    // https://en.cppreference.com/w/cpp/language/copy_elision.html
    // Allow for 2 constructions (with elision) or 3 (without).
    EXPECT_TRUE(LifecycleTracker::constructions == 2 || LifecycleTracker::constructions == 3);
    // Allow for 2 destructions (with elision) or 3 (without).
    EXPECT_TRUE(LifecycleTracker::destructions == 2 || LifecycleTracker::destructions == 3);
}

TEST_F(ExpectedTest, UnexpectedAssignment_ToUnexpectedExpected_AssignsError)
{
    std::expected<int, LifecycleTracker> ex(std::unexpect, LifecycleTracker(10));
    ex = std::unexpected(LifecycleTracker(100));
    EXPECT_FALSE(ex.has_value());
    EXPECT_EQ(100, ex.error().value);
}

// Modifiers

TEST_F(ExpectedTest, Emplace_OnValuedExpected_DestroysOldAndConstructsNewValue)
{
    LifecycleTracker::Reset();
    std::expected<LifecycleTracker, int> ex(std::in_place, 10);
    EXPECT_EQ(1, LifecycleTracker::constructions);

    ex.emplace(20);
    EXPECT_TRUE(ex.has_value());
    EXPECT_EQ(20, ex->value);
    EXPECT_EQ(2, LifecycleTracker::constructions);
    EXPECT_EQ(1, LifecycleTracker::destructions);
}

TEST_F(ExpectedTest, Emplace_OnUnexpectedExpected_DestroysErrorAndConstructsNewValue)
{
    LifecycleTracker::Reset();
    std::expected<LifecycleTracker, LifecycleTracker> ex(std::unexpect, 10);
    EXPECT_EQ(1, LifecycleTracker::constructions);

    ex.emplace(20);
    EXPECT_TRUE(ex.has_value());
    EXPECT_EQ(20, ex->value);
    EXPECT_EQ(2, LifecycleTracker::constructions);
    EXPECT_EQ(1, LifecycleTracker::destructions);
}

// Swap

TEST_F(ExpectedTest, Swap_WhenBothHaveValues_SwapsValues)
{
    std::expected<LifecycleTracker, int> ex1(LifecycleTracker(10));
    std::expected<LifecycleTracker, int> ex2(LifecycleTracker(20));
    ex1.swap(ex2);
    EXPECT_EQ(20, ex1->value);
    EXPECT_EQ(10, ex2->value);
}

TEST_F(ExpectedTest, Swap_WhenBothHaveErrors_SwapsErrors)
{
    std::expected<int, LifecycleTracker> ex1(std::unexpect, LifecycleTracker(10));
    std::expected<int, LifecycleTracker> ex2(std::unexpect, LifecycleTracker(20));
    ex1.swap(ex2);
    EXPECT_EQ(20, ex1.error().value);
    EXPECT_EQ(10, ex2.error().value);
}

TEST_F(ExpectedTest, Swap_WhenOneHasValueOneHasError_SwapsStatesAndContents)
{
    std::expected<LifecycleTracker, LifecycleTracker> ex1(LifecycleTracker(10));
    std::expected<LifecycleTracker, LifecycleTracker> ex2(std::unexpect, LifecycleTracker(20));
    ex1.swap(ex2);
    EXPECT_FALSE(ex1.has_value());
    EXPECT_EQ(20, ex1.error().value);
    EXPECT_TRUE(ex2.has_value());
    EXPECT_EQ(10, ex2->value);
}

// Observers

TEST_F(ExpectedTest, ArrowOperator_WhenHasValue_ReturnsPointerToValue)
{
    std::expected<MultiArgType, int> ex(std::in_place, 1, 2.0);
    EXPECT_EQ(1, ex->x);
}

FAIL_TEST_F(ExpectedTest, ArrowOperator_WhenHasError_Asserts)
{
    std::expected<int, int> ex(std::unexpect, 10);
    MAYBE_UNUSED volatile auto val = ex.operator->();
}

TEST_F(ExpectedTest, DereferenceOperator_WhenHasValue_ReturnsReferenceToValue)
{
    std::expected<int, int> ex(10);
    EXPECT_EQ(10, *ex);
    *ex = 20;
    EXPECT_EQ(20, *ex);
}

FAIL_TEST_F(ExpectedTest, DereferenceOperator_WhenHasError_Asserts)
{
    std::expected<int, int> ex(std::unexpect, 10);
    MAYBE_UNUSED volatile int val = *ex;
}

TEST_F(ExpectedTest, HasValue_WhenHasValue_ReturnsTrue)
{
    std::expected<int, int> ex(10);
    EXPECT_TRUE(ex.has_value());
    EXPECT_TRUE(static_cast<bool>(ex));
}

TEST_F(ExpectedTest, HasValue_WhenHasError_ReturnsFalse)
{
    std::expected<int, int> ex(std::unexpect, 10);
    EXPECT_FALSE(ex.has_value());
    EXPECT_FALSE(static_cast<bool>(ex));
}

TEST_F(ExpectedTest, Value_WhenHasValue_ReturnsReferenceToValue)
{
    std::expected<LifecycleTracker, int> ex(LifecycleTracker(10));
    EXPECT_EQ(10, ex.value().value);
}

FAIL_TEST_F(ExpectedTest, Value_WhenHasError_Panics)
{
    std::expected<int, int> ex(std::unexpect, 10);
    ex.value();
}

TEST_F(ExpectedTest, Error_WhenHasError_ReturnsReferenceToError)
{
    std::expected<int, int> ex(std::unexpect, 10);
    EXPECT_EQ(10, ex.error());
}

FAIL_TEST_F(ExpectedTest, Error_WhenHasValue_Asserts)
{
    std::expected<int, int> ex(10);
    ex.error();
}

TEST_F(ExpectedTest, ValueOr_WhenHasValue_ReturnsValue)
{
    std::expected<int, int> ex(10);
    EXPECT_EQ(10, ex.value_or(100));
}

TEST_F(ExpectedTest, ValueOr_WhenHasError_ReturnsDefaultValue)
{
    std::expected<int, int> ex(std::unexpect, 10);
    EXPECT_EQ(100, ex.value_or(100));
}

TEST_F(ExpectedTest, ErrorOr_WhenHasValue_ReturnsDefaultError)
{
    std::expected<int, int> ex(10);
    EXPECT_EQ(100, ex.error_or(100));
}

TEST_F(ExpectedTest, ErrorOr_WhenHasError_ReturnsError)
{
    std::expected<int, int> ex(std::unexpect, 10);
    EXPECT_EQ(10, ex.error_or(100));
}

//  Monadic Operations

auto int_to_double_expected(int x) { return std::expected<double, int>(x * 2.0); }
auto int_to_error_expected(int) { return std::expected<double, int>(std::unexpect, 99); }

TEST_F(ExpectedTest, AndThen_WhenHasValue_InvokesFunctionAndReturnsNewExpected)
{
    std::expected<int, int> ex(10);
    auto res = ex.and_then(int_to_double_expected);
    EXPECT_TRUE(res.has_value());
    EXPECT_EQ(20.0, *res);

    auto res2 = ex.and_then(int_to_error_expected);
    EXPECT_FALSE(res2.has_value());
    EXPECT_EQ(99, res2.error());
}

TEST_F(ExpectedTest, AndThen_WhenHasError_DoesNotInvokeFunctionAndPropagatesError)
{
    std::expected<int, int> ex(std::unexpect, 10);
    auto res = ex.and_then(int_to_double_expected);
    EXPECT_FALSE(res.has_value());
    EXPECT_EQ(10, res.error());
}

auto error_to_value_expected(int) { return std::expected<int, double>(99); }
auto error_to_error_expected(int e) { return std::expected<int, double>(std::unexpect, e * 2.0); }

TEST_F(ExpectedTest, OrElse_WhenHasValue_DoesNotInvokeFunctionAndPropagatesValue)
{
    std::expected<int, int> ex(10);
    auto res = ex.or_else(error_to_value_expected);
    EXPECT_TRUE(res.has_value());
    EXPECT_EQ(10, *res);
}

TEST_F(ExpectedTest, OrElse_WhenHasError_InvokesFunctionAndReturnsNewExpected)
{
    std::expected<int, int> ex(std::unexpect, 10);
    auto res = ex.or_else(error_to_value_expected);
    EXPECT_TRUE(res.has_value());
    EXPECT_EQ(99, *res);

    auto res2 = ex.or_else(error_to_error_expected);
    EXPECT_FALSE(res2.has_value());
    EXPECT_EQ(20.0, res2.error());
}

auto int_to_double(int x) { return x * 2.0; }

TEST_F(ExpectedTest, Transform_WhenHasValue_InvokesFunctionOnValueAndWrapsResult)
{
    std::expected<int, int> ex(10);
    auto res = ex.transform(int_to_double);
    EXPECT_TRUE(res.has_value());
    EXPECT_EQ(20.0, *res);
}

TEST_F(ExpectedTest, Transform_WhenHasError_DoesNotInvokeFunctionAndPropagatesError)
{
    std::expected<int, int> ex(std::unexpect, 10);
    auto res = ex.transform(int_to_double);
    EXPECT_FALSE(res.has_value());
    EXPECT_EQ(10, res.error());
}

auto int_err_to_double_err(int e) { return e * 2.0; }

TEST_F(ExpectedTest, TransformError_WhenHasValue_DoesNotInvokeFunctionAndPropagatesValue)
{
    std::expected<int, int> ex(10);
    auto res = ex.transform_error(int_err_to_double_err);
    EXPECT_TRUE(res.has_value());
    EXPECT_EQ(10, *res);
}

TEST_F(ExpectedTest, TransformError_WhenHasError_InvokesFunctionOnErrorAndWrapsResult)
{
    std::expected<int, int> ex(std::unexpect, 10);
    auto res = ex.transform_error(int_err_to_double_err);
    EXPECT_FALSE(res.has_value());
    EXPECT_EQ(20.0, res.error());
}

// Comparison

TEST_F(ExpectedTest, OperatorEqualsExpected_WhenBothHaveEqualValues_ReturnsTrue)
{
    std::expected<int, int> ex1(10);
    std::expected<int, int> ex2(10);
    EXPECT_TRUE(ex1 == ex2);
}

TEST_F(ExpectedTest, OperatorEqualsExpected_WhenBothHaveEqualErrors_ReturnsTrue)
{
    std::expected<int, int> ex1(std::unexpect, 20);
    std::expected<int, int> ex2(std::unexpect, 20);
    EXPECT_TRUE(ex1 == ex2);
}

TEST_F(ExpectedTest, OperatorEqualsExpected_WhenOneHasValueOneHasError_ReturnsFalse)
{
    std::expected<int, int> ex1(10);
    std::expected<int, int> ex2(std::unexpect, 10);
    EXPECT_FALSE(ex1 == ex2);
}

TEST_F(ExpectedTest, OperatorEqualsValue_WhenHasEqualValue_ReturnsTrue)
{
    std::expected<int, int> ex(10);
    EXPECT_TRUE(ex == 10);
}

TEST_F(ExpectedTest, OperatorEqualsValue_WhenHasError_ReturnsFalse)
{
    std::expected<int, int> ex(std::unexpect, 10);
    EXPECT_FALSE(ex == 10);
}

TEST_F(ExpectedTest, OperatorEqualsUnexpected_WhenHasEqualError_ReturnsTrue)
{
    std::expected<int, int> ex(std::unexpect, 10);
    EXPECT_TRUE(ex == std::unexpected(10));
}

TEST_F(ExpectedTest, OperatorEqualsUnexpected_WhenHasValue_ReturnsFalse)
{
    std::expected<int, int> ex(10);
    EXPECT_FALSE(ex == std::unexpected(10));
}

//------------------------------------------------------------------------------
// std::expected<void, E> Tests
//------------------------------------------------------------------------------

// Constructors & Assignment

TEST_F(ExpectedTest, VoidDefaultConstructor_Always_ConstructsWithValue)
{
    std::expected<void, int> ex;
    EXPECT_TRUE(ex.has_value());
}

TEST_F(ExpectedTest, VoidInPlaceValueConstructor_Always_ConstructsWithValue)
{
    std::expected<void, int> ex(std::in_place);
    EXPECT_TRUE(ex.has_value());
}

TEST_F(ExpectedTest, VoidCopyAssignment_FromValueToValue_DoesNothing)
{
    std::expected<void, int> ex1, ex2;
    ex1 = ex2;
    EXPECT_TRUE(ex1.has_value());
}

TEST_F(ExpectedTest, VoidCopyAssignment_FromValueToError_DestroysError)
{
    LifecycleTracker::Reset();
    std::expected<void, LifecycleTracker> ex1(std::unexpect, LifecycleTracker(10));
    std::expected<void, LifecycleTracker> ex2;
    LifecycleTracker::Reset();

    ex1 = ex2;
    EXPECT_TRUE(ex1.has_value());
    EXPECT_EQ(1, LifecycleTracker::destructions);
}

// Modifiers

TEST_F(ExpectedTest, VoidEmplace_OnValuedExpected_DoesNothing)
{
    std::expected<void, int> ex;
    ex.emplace();
    EXPECT_TRUE(ex.has_value());
}

TEST_F(ExpectedTest, VoidEmplace_OnUnexpectedExpected_DestroysErrorAndSetsValueState)
{
    LifecycleTracker::Reset();
    std::expected<void, LifecycleTracker> ex(std::unexpect, LifecycleTracker(10));
    LifecycleTracker::Reset();
    ex.emplace();
    EXPECT_TRUE(ex.has_value());
    EXPECT_EQ(1, LifecycleTracker::destructions);
}

// Observers

TEST_F(ExpectedTest, VoidArrowOperator_WhenHasValue_ReturnsNullptr)
{
    std::expected<void, int> ex;
    EXPECT_EQ(nullptr, ex.operator->());
}

TEST_F(ExpectedTest, VoidDereferenceOperator_WhenHasValue_DoesNothing)
{
    std::expected<void, int> ex;
    *ex;  // Should not crash
    SUCCEED();
}

TEST_F(ExpectedTest, VoidValue_WhenHasValue_DoesNothing)
{
    std::expected<void, int> ex;
    ex.value();  // Should not crash
    SUCCEED();
}

FAIL_TEST_F(ExpectedTest, VoidValue_WhenHasError_Panics)
{
    std::expected<void, int> ex(std::unexpect, 10);
    ex.value();
}

// Monadic Operations
auto no_arg_func_returns_double() { return 3.14; }

TEST_F(
    ExpectedTest, VoidTransform_WhenHasValue_InvokesFunctionAndReturnsExpectedWithTransformedValue
)
{
    std::expected<void, int> ex;
    auto res = ex.transform(no_arg_func_returns_double);
    EXPECT_TRUE(res.has_value());
    EXPECT_EQ(3.14, *res);
}

// Comparison

TEST_F(ExpectedTest, VoidOperatorEqualsExpected_WhenBothHaveValue_ReturnsTrue)
{
    std::expected<void, int> ex1;
    std::expected<void, int> ex2;
    EXPECT_TRUE(ex1 == ex2);
}
