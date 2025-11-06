#include <limits.hpp>
#include <test_module/test.hpp>

class LimitsTest : public TestGroupBase
{
};

TEST_F(LimitsTest, Digits10Calculation)
{
    EXPECT_EQ(2, std::numeric_limits<u8>::digits10);
    EXPECT_EQ(4, std::numeric_limits<u16>::digits10);
    EXPECT_EQ(9, std::numeric_limits<u32>::digits10);
    EXPECT_EQ(19, std::numeric_limits<u64>::digits10);
}

TEST_F(LimitsTest, UnsignedIntegerLimits)
{
    EXPECT_EQ(0, std::numeric_limits<u8>::min());
    EXPECT_EQ(255, std::numeric_limits<u8>::max());
    EXPECT_EQ(8, std::numeric_limits<u8>::digits);
    EXPECT_TRUE(std::numeric_limits<u8>::is_modulo);
    EXPECT_FALSE(std::numeric_limits<u8>::is_signed);
    EXPECT_EQ(0, std::numeric_limits<u16>::min());
    EXPECT_EQ(65535, std::numeric_limits<u16>::max());
    EXPECT_EQ(16, std::numeric_limits<u16>::digits);
    EXPECT_EQ(0u, std::numeric_limits<u32>::min());
    EXPECT_EQ(4294967295u, std::numeric_limits<u32>::max());
    EXPECT_EQ(32, std::numeric_limits<u32>::digits);
    EXPECT_EQ(0ull, std::numeric_limits<u64>::min());
    EXPECT_EQ(18446744073709551615ull, std::numeric_limits<u64>::max());
    EXPECT_EQ(64, std::numeric_limits<u64>::digits);
}

TEST_F(LimitsTest, SignedIntegerLimits)
{
    EXPECT_EQ(-128, std::numeric_limits<i8>::min());
    EXPECT_EQ(127, std::numeric_limits<i8>::max());
    EXPECT_EQ(-128, std::numeric_limits<i8>::lowest());
    EXPECT_EQ(7, std::numeric_limits<i8>::digits);
    EXPECT_FALSE(std::numeric_limits<i8>::is_modulo);
    EXPECT_TRUE(std::numeric_limits<i8>::is_signed);
    EXPECT_EQ(-32768, std::numeric_limits<i16>::min());
    EXPECT_EQ(32767, std::numeric_limits<i16>::max());
    EXPECT_EQ(15, std::numeric_limits<i16>::digits);
    EXPECT_EQ(-2147483648, std::numeric_limits<i32>::min());
    EXPECT_EQ(2147483647, std::numeric_limits<i32>::max());
    EXPECT_EQ(31, std::numeric_limits<i32>::digits);
    EXPECT_EQ(-9223372036854775807LL - 1, std::numeric_limits<i64>::min());
    EXPECT_EQ(9223372036854775807LL, std::numeric_limits<i64>::max());
    EXPECT_EQ(63, std::numeric_limits<i64>::digits);
}

TEST_F(LimitsTest, BoolLimits)
{
    EXPECT_EQ(false, std::numeric_limits<bool>::min());
    EXPECT_EQ(true, std::numeric_limits<bool>::max());
    EXPECT_EQ(false, std::numeric_limits<bool>::lowest());
    EXPECT_EQ(1, std::numeric_limits<bool>::digits);
    EXPECT_EQ(0, std::numeric_limits<bool>::digits10);
    EXPECT_FALSE(std::numeric_limits<bool>::is_signed);
    EXPECT_TRUE(std::numeric_limits<bool>::is_integer);
    EXPECT_FALSE(std::numeric_limits<bool>::is_modulo);
}

TEST_F(LimitsTest, CvQualifiedTypes)
{
    EXPECT_EQ(std::numeric_limits<int>::min(), std::numeric_limits<const int>::min());
    EXPECT_EQ(std::numeric_limits<int>::max(), std::numeric_limits<const int>::max());
    EXPECT_EQ(std::numeric_limits<int>::digits, std::numeric_limits<const int>::digits);
    EXPECT_EQ(std::numeric_limits<unsigned>::min(), std::numeric_limits<volatile unsigned>::min());
    EXPECT_EQ(std::numeric_limits<unsigned>::max(), std::numeric_limits<volatile unsigned>::max());
    EXPECT_EQ(
        std::numeric_limits<long>::is_signed, std::numeric_limits<const volatile long>::is_signed
    );
    EXPECT_EQ(std::numeric_limits<long>::digits, std::numeric_limits<const volatile long>::digits);
}

TEST_F(LimitsTest, SpecializationFlags)
{
    EXPECT_TRUE(std::numeric_limits<int>::is_specialized);
    EXPECT_TRUE(std::numeric_limits<unsigned>::is_specialized);
    EXPECT_TRUE(std::numeric_limits<bool>::is_specialized);
    EXPECT_TRUE(std::numeric_limits<int>::is_integer);
    EXPECT_TRUE(std::numeric_limits<unsigned>::is_integer);
    EXPECT_TRUE(std::numeric_limits<bool>::is_integer);
    EXPECT_TRUE(std::numeric_limits<int>::is_exact);
    EXPECT_TRUE(std::numeric_limits<unsigned>::is_exact);
    EXPECT_TRUE(std::numeric_limits<int>::is_bounded);
    EXPECT_TRUE(std::numeric_limits<unsigned>::is_bounded);
    EXPECT_EQ(2, std::numeric_limits<int>::radix);
    EXPECT_EQ(2, std::numeric_limits<unsigned>::radix);
}

TEST_F(LimitsTest, FloatConstantsForIntegers)
{
    EXPECT_EQ(0, std::numeric_limits<int>::min_exponent);
    EXPECT_EQ(0, std::numeric_limits<int>::max_exponent);
    EXPECT_FALSE(std::numeric_limits<int>::has_infinity);
    EXPECT_FALSE(std::numeric_limits<int>::has_quiet_NaN);
    EXPECT_FALSE(std::numeric_limits<int>::has_signaling_NaN);
    EXPECT_EQ(std::denorm_absent, std::numeric_limits<int>::has_denorm);
    EXPECT_FALSE(std::numeric_limits<int>::is_iec559);
    EXPECT_EQ(std::round_toward_zero, std::numeric_limits<int>::round_style);
}

TEST_F(LimitsTest, FloatFunctionsForIntegers)
{
    EXPECT_EQ(0, std::numeric_limits<int>::epsilon());
    EXPECT_EQ(0, std::numeric_limits<int>::round_error());
    EXPECT_EQ(0, std::numeric_limits<int>::infinity());
    EXPECT_EQ(0, std::numeric_limits<int>::quiet_NaN());
    EXPECT_EQ(0, std::numeric_limits<int>::signaling_NaN());
    EXPECT_EQ(0, std::numeric_limits<int>::denorm_min());
    EXPECT_EQ(0u, std::numeric_limits<unsigned>::epsilon());
    EXPECT_EQ(0u, std::numeric_limits<unsigned>::round_error());
}

TEST_F(LimitsTest, UnsignedMaxCalculation)
{
    u8 u8_expected = static_cast<u8>(0) - 1;
    EXPECT_EQ(u8_expected, std::numeric_limits<u8>::max());
    u16 u16_expected = static_cast<u16>(0) - 1;
    EXPECT_EQ(u16_expected, std::numeric_limits<u16>::max());
    u32 u32_expected = static_cast<u32>(0) - 1;
    EXPECT_EQ(u32_expected, std::numeric_limits<u32>::max());
    u64 u64_expected = static_cast<u64>(0) - 1;
    EXPECT_EQ(u64_expected, std::numeric_limits<u64>::max());
}

TEST_F(LimitsTest, SignedBitManipulation)
{
    EXPECT_EQ(static_cast<int8_t>(0x80), std::numeric_limits<int8_t>::min());
    EXPECT_EQ(static_cast<int8_t>(0x7F), std::numeric_limits<int8_t>::max());
    EXPECT_EQ(static_cast<int16_t>(0x8000), std::numeric_limits<int16_t>::min());
    EXPECT_EQ(static_cast<int16_t>(0x7FFF), std::numeric_limits<int16_t>::max());
}

TEST_F(LimitsTest, DigitsVsSize)
{
    EXPECT_EQ(static_cast<int>(sizeof(uint8_t) * 8), std::numeric_limits<uint8_t>::digits);
    EXPECT_EQ(static_cast<int>(sizeof(uint16_t) * 8), std::numeric_limits<uint16_t>::digits);
    EXPECT_EQ(static_cast<int>(sizeof(uint32_t) * 8), std::numeric_limits<uint32_t>::digits);
    EXPECT_EQ(static_cast<int>(sizeof(uint64_t) * 8), std::numeric_limits<uint64_t>::digits);
    EXPECT_EQ(static_cast<int>(sizeof(int8_t) * 8 - 1), std::numeric_limits<int8_t>::digits);
    EXPECT_EQ(static_cast<int>(sizeof(int16_t) * 8 - 1), std::numeric_limits<int16_t>::digits);
    EXPECT_EQ(static_cast<int>(sizeof(int32_t) * 8 - 1), std::numeric_limits<int32_t>::digits);
    EXPECT_EQ(static_cast<int>(sizeof(int64_t) * 8 - 1), std::numeric_limits<int64_t>::digits);
}

TEST_F(LimitsTest, DefaultSpecialization)
{
    struct CustomType {
    };
    EXPECT_FALSE(std::numeric_limits<CustomType>::is_specialized);
    EXPECT_EQ(0, std::numeric_limits<CustomType>::digits);
    EXPECT_EQ(0, std::numeric_limits<CustomType>::digits10);
    EXPECT_FALSE(std::numeric_limits<CustomType>::is_signed);
    EXPECT_FALSE(std::numeric_limits<CustomType>::is_integer);
}
