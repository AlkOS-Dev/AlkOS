#include <types.h>
#include <data_structures/bitfield.hpp>
#include <test_module/test.hpp>

class BitFieldTest : public TestGroupBase
{
};

CREATE_BITFIELD(TestBitField1, FieldA, 3, FieldB, 5, FieldC, 8);

CREATE_BITFIELD(TestBitField2, FlagX, 1, FlagY, 1, Counter, 6);

CREATE_BITFIELD(StatusReg, Ready, 1, Error, 1, ErrorCode, 6, Reserved, 24);

static_assert(sizeof(StatusReg) == sizeof(u32));
static_assert(StatusReg::kTotalBits == 32);
static_assert(sizeof(TestBitField1) == sizeof(u16));
static_assert(sizeof(TestBitField2) == sizeof(u8));

struct RawStatus {
    u32 flags;
};

TEST_F(BitFieldTest, BitFieldConstruction)
{
    TestBitField1 bf1{static_cast<u16>(0x1FF8)};
    EXPECT_EQ(0, bf1.get<TestBitField1::FieldA>());
    EXPECT_EQ(31, bf1.get<TestBitField1::FieldB>());
    EXPECT_EQ(31, bf1.get<TestBitField1::FieldC>());

    TestBitField2 bf2;
    EXPECT_EQ(0, bf2.get<TestBitField2::FlagX>());
    EXPECT_EQ(0, bf2.get<TestBitField2::FlagY>());
    EXPECT_EQ(0, bf2.get<TestBitField2::Counter>());
}

TEST_F(BitFieldTest, BitFieldIntegerConstruction)
{
    u32 val1 = 0x01000003;  // Binary ...00011 (Ready=1, Error=1)
    StatusReg reg(val1);

    EXPECT_EQ(1, reg.get<StatusReg::Ready>());
    EXPECT_EQ(1, reg.get<StatusReg::Error>());

    RawStatus raw{0xFFFFFFFF};
    reg = raw;

    EXPECT_EQ(63, reg.get<StatusReg::ErrorCode>());
    EXPECT_EQ(16777215, reg.get<StatusReg::Reserved>());

    reg.set<StatusReg::Ready>(0);
    reg.set<StatusReg::Error>(0);

    u32 val = static_cast<u32>(reg);
    EXPECT_EQ(0xFFFFFFFC, val);
}

TEST_F(BitFieldTest, BitFieldSetGet)
{
    TestBitField1 bf1;
    bf1.set<TestBitField1::FieldA>(5);
    bf1.set<TestBitField1::FieldB>(17);
    bf1.set<TestBitField1::FieldC>(255);

    EXPECT_EQ(5, bf1.get<TestBitField1::FieldA>());
    EXPECT_EQ(17, bf1.get<TestBitField1::FieldB>());
    EXPECT_EQ(255, bf1.get<TestBitField1::FieldC>());

    TestBitField2 bf2;
    bf2.set<TestBitField2::FlagX>(1);
    bf2.set<TestBitField2::FlagY>(0);
    bf2.set<TestBitField2::Counter>(42);

    EXPECT_EQ(1, bf2.get<TestBitField2::FlagX>());
    EXPECT_EQ(0, bf2.get<TestBitField2::FlagY>());
    EXPECT_EQ(42, bf2.get<TestBitField2::Counter>());
}
