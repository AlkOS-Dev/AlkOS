#include <data_structures/bit_array.hpp>
#include <test_module/test.hpp>

#include <string.h>

class BitArrayTest : public TestGroupBase
{
};

TEST_F(BitArrayTest, BasicSetGet)
{
    data_structures::BitArray<32> bits;

    bits.SetTrue(0);
    EXPECT_TRUE(bits.Get(0));

    bits.SetFalse(0);
    EXPECT_FALSE(bits.Get(0));

    bits.Set(5, true);
    EXPECT_TRUE(bits.Get(5));

    bits.Set(5, false);
    EXPECT_FALSE(bits.Get(5));
}

TEST_F(BitArrayTest, CrossBoundary)
{
    data_structures::BitArray<64> bits;

    bits.SetTrue(31);
    bits.SetTrue(32);

    EXPECT_TRUE(bits.Get(31));
    EXPECT_TRUE(bits.Get(32));
    EXPECT_FALSE(bits.Get(30));
    EXPECT_FALSE(bits.Get(33));
}

TEST_F(BitArrayTest, GetSetRange_BoundariesCrossing)
{
    data_structures::BitArray<64> bits;

    // Case 1: No boundary crossing - 5 bits at offset 2 (bits 2-6, within first byte)
    bits.SetRange<u8, 2, 5>(0x15);  // 0b10101
    EXPECT_EQ(0x15, (bits.GetRange<u8, 2, 5>()));

    // Verify unaffected bits
    EXPECT_EQ(0, (bits.GetRange<u8, 0, 2>()));
    EXPECT_EQ(0, (bits.GetRange<u8, 7, 1>()));

    // Case 2: One boundary crossing - 12 bits at offset 4 (bits 4-15, crosses at bit 8)
    bits.SetRange<u16, 4, 12>(0xABC);  // 12 bits spanning 2 bytes
    EXPECT_EQ(0xABC, (bits.GetRange<u16, 4, 12>()));

    // Case 3: Multiple boundary crossings - 25 bits at offset 20 (bits 20-44, crosses 3 byte
    // boundaries) Crosses at bits 24, 32, 40
    bits.SetRange<u32, 20, 25>(0x1FEDCBA);
    EXPECT_EQ(0x1FEDCBA, (bits.GetRange<u32, 20, 25>()));

    // Verify unaffected bits
    EXPECT_EQ(0, (bits.GetRange<u8, 16, 4>()));
    EXPECT_EQ(0, (bits.GetRange<u8, 45, 3>()));
}

TEST_F(BitArrayTest, Size)
{
    data_structures::BitArray<100> bits;
    EXPECT_EQ(bits.Size(), 100_size);

    data_structures::BitArray<32> smallBits;
    EXPECT_EQ(smallBits.Size(), 32_size);
}

TEST_F(BitArrayTest, SetAll)
{
    data_structures::BitArray<100> bits;

    bits.SetTrue(50);
    bits.SetAll(false);
    for (size_t i = 0; i < bits.Size(); ++i) {
        EXPECT_FALSE(bits.Get(i));
    }

    bits.SetAll(true);
    for (size_t i = 0; i < bits.Size(); ++i) {
        EXPECT_TRUE(bits.Get(i));
    }
}

TEST_F(BitArrayTest, EdgeCases)
{
    data_structures::BitArray<1> singleBit;
    singleBit.SetTrue(0);
    EXPECT_TRUE(singleBit.Get(0));

    data_structures::BitArray<33> oddSized;
    oddSized.SetTrue(32);
    EXPECT_TRUE(oddSized.Get(32));
    EXPECT_FALSE(oddSized.Get(31));
}

TEST_F(BitArrayTest, AlternatingPattern)
{
    data_structures::BitArray<64> bits;

    for (size_t i = 0; i < bits.Size(); ++i) {
        if (i % 2 == 0) {
            bits.SetTrue(i);
        }
    }

    for (size_t i = 0; i < bits.Size(); ++i) {
        if (i % 2 == 0) {
            EXPECT_TRUE(bits.Get(i));
        } else {
            EXPECT_FALSE(bits.Get(i));
        }
    }
}
