#include <bit.hpp>
#include <test_module/test.hpp>

class BitManipTest : public TestGroupBase
{
    protected:
    u8 val8{};
    u16 val16{};
    u32 val32{};
    u64 val64{};
};

TEST_F(BitManipTest, Constants)
{
    EXPECT_EQ(kBitMask4, static_cast<u64>(0xF));
    EXPECT_EQ(kBitMask8, static_cast<u64>(0xFF));
    EXPECT_EQ(kBitMask16, static_cast<u64>(0xFFFF));
    EXPECT_EQ(kBitMask32, static_cast<u64>(0xFFFFFFFF));
    EXPECT_EQ(kBitMask64, static_cast<u64>(0xFFFFFFFFFFFFFFFF));

    EXPECT_EQ(kMsb<u8>, static_cast<u8>(0x80));
    EXPECT_EQ(kMsb<u16>, static_cast<u16>(0x8000));
    EXPECT_EQ(kMsb<u32>, static_cast<u32>(0x80000000));
    EXPECT_EQ(kMsb<u64>, static_cast<u64>(0x8000000000000000));
}

TEST_F(BitManipTest, SetBit)
{
    // Test u8
    SetBit(val8, 0);
    EXPECT_EQ(val8, 1);
    SetBit(val8, 7);
    EXPECT_EQ(val8, kMsb<u8> | kLsb<u8>);

    // Test u16
    SetBit(val16, 0);
    EXPECT_EQ(val16, 1);
    SetBit(val16, 15);
    EXPECT_EQ(val16, kMsb<u16> | kLsb<u16>);
}

TEST_F(BitManipTest, ClearBit)
{
    // Test u8
    val8 = kFullMask<u8>;
    ClearBit(val8, 0);
    EXPECT_EQ(val8, 0xFE);
    ClearBit(val8, 7);
    EXPECT_EQ(val8, 0x7E);

    // Test u16
    val16 = kFullMask<u16>;
    ClearBit(val16, 0);
    EXPECT_EQ(val16, 0xFFFE);
    ClearBit(val16, 15);
    EXPECT_EQ(val16, 0x7FFE);
}

TEST_F(BitManipTest, SwitchBit)
{
    // Test u8
    SwitchBit(val8, 0);
    EXPECT_EQ(val8, 1);
    SwitchBit(val8, 0);
    EXPECT_EQ(val8, 0);
    SwitchBit(val8, 7);
    EXPECT_EQ(val8, kMsb<u8>);
    SwitchBit(val8, 7);
    EXPECT_EQ(val8, 0);

    // Test u16
    SwitchBit(val16, 0);
    EXPECT_EQ(val16, 1);
    SwitchBit(val16, 0);
    EXPECT_EQ(val16, 0);
    SwitchBit(val16, 15);
    EXPECT_EQ(val16, kMsb<u16>);
    SwitchBit(val16, 15);
    EXPECT_EQ(val16, 0);
}

TEST_F(BitManipTest, SetBitValue)
{
    // Test u8
    SetBitValue(val8, 0, true);
    EXPECT_EQ(val8, 1);
    SetBitValue(val8, 0, false);
    EXPECT_EQ(val8, 0);
    SetBitValue(val8, 7, true);
    EXPECT_EQ(val8, kMsb<u8>);

    // Test u16
    SetBitValue(val16, 0, true);
    EXPECT_EQ(val16, 1);
    SetBitValue(val16, 15, true);
    EXPECT_EQ(val16, kMsb<u16> | kLsb<u16>);
    SetBitValue(val16, 0, false);
    EXPECT_EQ(val16, kMsb<u16>);
}

TEST_F(BitManipTest, CountlZero)
{
    EXPECT_EQ(std::countl_zero(static_cast<u8>(0)), 8);
    EXPECT_EQ(std::countl_zero(kFullMask<u8>), 0);
    EXPECT_EQ(std::countl_zero(kMsb<u8>), 0);
    EXPECT_EQ(std::countl_zero(kLsb<u8>), 7);
    EXPECT_EQ(std::countl_zero(static_cast<u16>(0x0F00)), 4);
    EXPECT_EQ(std::countl_zero(0x00001234u), 19);
    EXPECT_EQ(std::countl_zero(0x000000000000ABCDull), 48);
}

TEST_F(BitManipTest, CountlOne)
{
    EXPECT_EQ(std::countl_one(static_cast<u8>(0)), 0);
    EXPECT_EQ(std::countl_one(kFullMask<u8>), 8);
    EXPECT_EQ(std::countl_one(kMsb<u8>), 1);
    EXPECT_EQ(std::countl_one(kLsb<u8>), 0);
    EXPECT_EQ(std::countl_one(static_cast<u16>(0xF0F0)), 4);
    EXPECT_EQ(std::countl_one(0xFFF01234u), 12);
    EXPECT_EQ(std::countl_one(0xFFFFFF000000ABCDull), 24);
}

TEST_F(BitManipTest, CountrZero)
{
    EXPECT_EQ(std::countr_zero(static_cast<u8>(0)), 8);
    EXPECT_EQ(std::countr_zero(kFullMask<u8>), 0);
    EXPECT_EQ(std::countr_zero(kMsb<u8>), 7);
    EXPECT_EQ(std::countr_zero(kLsb<u8>), 0);
    EXPECT_EQ(std::countr_zero(static_cast<u16>(0x0F00)), 8);
    EXPECT_EQ(std::countr_zero(0x12340000u), 18);
    EXPECT_EQ(std::countr_zero(0xABCD000000000000ull), 48);
}

TEST_F(BitManipTest, CountrOne)
{
    EXPECT_EQ(std::countr_one(static_cast<u8>(0)), 0);
    EXPECT_EQ(std::countr_one(kFullMask<u8>), 8);
    EXPECT_EQ(std::countr_one(kMsb<u8>), 0);
    EXPECT_EQ(std::countr_one(kLsb<u8>), 1);
    EXPECT_EQ(std::countr_one(static_cast<u16>(0x00FF)), 8);
    EXPECT_EQ(std::countr_one(0x0000FFFFu), 16);
    EXPECT_EQ(std::countr_one(0x00000000FFFFFFFFull), 32);
}

TEST_F(BitManipTest, Popcount)
{
    EXPECT_EQ(std::popcount(static_cast<u8>(0)), 0);
    EXPECT_EQ(std::popcount(kFullMask<u8>), 8);
    EXPECT_EQ(std::popcount(static_cast<u8>(0b10101010)), 4);
    EXPECT_EQ(std::popcount(static_cast<u16>(0xF0F0)), 8);
    EXPECT_EQ(std::popcount(0xAAAAAAAAu), 16);
    EXPECT_EQ(std::popcount(0x5555555555555555ull), 32);
}

TEST_F(BitManipTest, BitWidth)
{
    EXPECT_EQ(std::bit_width(static_cast<u8>(0)), 0);
    EXPECT_EQ(std::bit_width(static_cast<u8>(1)), 1);
    EXPECT_EQ(std::bit_width(static_cast<u8>(2)), 2);
    EXPECT_EQ(std::bit_width(static_cast<u8>(3)), 2);
    EXPECT_EQ(std::bit_width(static_cast<u8>(128)), 8);
    EXPECT_EQ(std::bit_width(kFullMask<u8>), 8);
    EXPECT_EQ(std::bit_width(0x7FFFFFFFu), 31);
    EXPECT_EQ(std::bit_width(kMsb<u32>), 32);
}

TEST_F(BitManipTest, BitFloor)
{
    EXPECT_EQ(std::bit_floor(static_cast<u8>(0)), 0);
    EXPECT_EQ(std::bit_floor(static_cast<u8>(1)), 1);
    EXPECT_EQ(std::bit_floor(static_cast<u8>(3)), 2);
    EXPECT_EQ(std::bit_floor(static_cast<u8>(127)), 64);
    EXPECT_EQ(std::bit_floor(kFullMask<u8>), kMsb<u8>);
    EXPECT_EQ(std::bit_floor(0xABCDEF01u), 0x80000000u);
    EXPECT_EQ(std::bit_floor(kFullMask<u32>), kMsb<u32>);
}

TEST_F(BitManipTest, BitCeil)
{
    EXPECT_EQ(std::bit_ceil(static_cast<u8>(0)), 1);
    EXPECT_EQ(std::bit_ceil(static_cast<u8>(1)), 1);
    EXPECT_EQ(std::bit_ceil(static_cast<u8>(2)), 2);
    EXPECT_EQ(std::bit_ceil(static_cast<u8>(3)), 4);
    EXPECT_EQ(std::bit_ceil(static_cast<u8>(127)), 128);
    EXPECT_EQ(std::bit_ceil(static_cast<u8>(128)), 128);

    EXPECT_EQ(std::bit_ceil(0x7FFFFFFFu), kMsb<u32>);
    EXPECT_EQ(std::bit_ceil(kMsb<u32>), kMsb<u32>);
}

class AlignementTest : public TestGroupBase
{
};

TEST_F(AlignementTest, IsAligned_GivenUnalignedValue_ReturnsFalse)
{
    EXPECT_FALSE(IsAligned(3u, 4));
    EXPECT_FALSE(IsAligned(15u, 8));
}

TEST_F(AlignementTest, IsAligned_GivenAlignedValue_ReturnsTrue)
{
    EXPECT_TRUE(IsAligned(4u, 4));
    EXPECT_TRUE(IsAligned(16u, 8));
}

TEST_F(AlignementTest, IsAligned_GivenUnalignedPointer_ReturnsFalse)
{
    auto unaligned_ptr = reinterpret_cast<byte *>(uintptr_t(0x1003));
    EXPECT_FALSE(IsAligned(unaligned_ptr, 4));
}

TEST_F(AlignementTest, IsAligned_GivenAlignedPointer_ReturnsTrue)
{
    auto aligned_ptr = reinterpret_cast<byte *>(uintptr_t(0x1004));
    EXPECT_TRUE(IsAligned(aligned_ptr, 4));
}

TEST_F(AlignementTest, AlignDown_GivenUnalignedValue_ReturnsAlignedValue)
{
    EXPECT_EQ(AlignDown(0x101u, 16), 0x100u);
    EXPECT_EQ(AlignDown(0x1FFu, 16), 0x1F0u);
}

TEST_F(AlignementTest, AlignDown_GivenAlignedValue_ReturnsSameValue)
{
    EXPECT_EQ(AlignDown(0x100u, 16), 0x100u);
}

TEST_F(AlignementTest, AlignUp_GivenUnalignedValue_ReturnsAlignedValue)
{
    EXPECT_EQ(AlignUp(0x101u, 16), 0x110u);
    EXPECT_EQ(AlignUp(0x1F1u, 16), 0x200u);
}

TEST_F(AlignementTest, AlignUp_GivenAlignedValue_ReturnsSameValue)
{
    EXPECT_EQ(AlignUp(0x100u, 16), 0x100u);
}

TEST_F(AlignementTest, AlignUp_GivenUnalignedPointer_ReturnsAlignedPointer)
{
    auto unaligned_ptr = reinterpret_cast<char *>(uintptr_t(0x1003));
    auto aligned_ptr   = AlignUp(unaligned_ptr, 4);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(aligned_ptr), uintptr_t(0x1004));
}

TEST_F(AlignementTest, AlignUp_GivenAlignedPointer_ReturnsSamePointer)
{
    auto aligned_ptr = reinterpret_cast<char *>(uintptr_t(0x1004));
    EXPECT_EQ(AlignUp(aligned_ptr, 4), aligned_ptr);
}

TEST_F(AlignementTest, IsAligned_GivenNonPowerOfTwoAlignment_ReturnsCorrectResult)
{
    EXPECT_TRUE(IsAligned(18u, 6));
    EXPECT_TRUE(IsAligned(30u, 10));
    EXPECT_FALSE(IsAligned(17u, 6));
    EXPECT_FALSE(IsAligned(29u, 10));
}
TEST_F(AlignementTest, IsAligned_GivenNonPowerOfTwoAlignedPointer_ReturnsCorrectResult)
{
    auto aligned_ptr   = reinterpret_cast<byte *>(uintptr_t(24));
    auto unaligned_ptr = reinterpret_cast<byte *>(uintptr_t(25));
    EXPECT_TRUE(IsAligned(aligned_ptr, 12));
    EXPECT_FALSE(IsAligned(unaligned_ptr, 12));
}
TEST_F(AlignementTest, AlignDown_GivenNonPowerOfTwoAlignment_ReturnsAlignedValue)
{
    EXPECT_EQ(AlignDown(23u, 10), 20u);
    EXPECT_EQ(AlignDown(17u, 5), 15u);
    EXPECT_EQ(AlignDown(100u, 3), 99u);
}
TEST_F(AlignementTest, AlignDown_GivenNonPowerOfTwoAlignedPointer_ReturnsCorrectPointer)
{
    auto unaligned_ptr = reinterpret_cast<char *>(uintptr_t(128));
    auto aligned_ptr   = AlignDown(unaligned_ptr, 13);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(aligned_ptr), uintptr_t(117));
}
TEST_F(AlignementTest, AlignUp_GivenNonPowerOfTwoAlignment_ReturnsAlignedValue)
{
    EXPECT_EQ(AlignUp(21u, 10), 30u);
    EXPECT_EQ(AlignUp(16u, 5), 20u);
    EXPECT_EQ(AlignUp(99u, 3), 99u);
}
TEST_F(AlignementTest, AlignUp_GivenNonPowerOfTwoAlignedPointer_ReturnsCorrectPointer)
{
    auto unaligned_ptr = reinterpret_cast<char *>(uintptr_t(118));
    auto aligned_ptr   = AlignUp(unaligned_ptr, 13);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(aligned_ptr), uintptr_t(130));
}
