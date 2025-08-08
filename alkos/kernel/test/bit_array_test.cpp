#include <extensions/bit_array.hpp>
#include <test_module/test.hpp>

class BitArrayTest : public TestGroupBase
{
};

TEST_F(BitArrayTest, BasicSetGet)
{
    BitArray<32> bits;

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
    BitArray<64> bits;

    bits.SetTrue(31);
    bits.SetTrue(32);

    EXPECT_TRUE(bits.Get(31));
    EXPECT_TRUE(bits.Get(32));
    EXPECT_FALSE(bits.Get(30));
    EXPECT_FALSE(bits.Get(33));
}

TEST_F(BitArrayTest, Size)
{
    BitArray<100> bits;
    EXPECT_EQ(bits.Size(), 100_size);

    BitArray<32> smallBits;
    EXPECT_EQ(smallBits.Size(), 32_size);
}

TEST_F(BitArrayTest, SetAll)
{
    BitArray<100> bits;

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
    BitArray<1> singleBit;
    singleBit.SetTrue(0);
    EXPECT_TRUE(singleBit.Get(0));

    BitArray<33> oddSized;
    oddSized.SetTrue(32);
    EXPECT_TRUE(oddSized.Get(32));
    EXPECT_FALSE(oddSized.Get(31));
}

TEST_F(BitArrayTest, AlternatingPattern)
{
    BitArray<64> bits;

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
