#include <limits.hpp>
#include <template/type_traits.hpp>
#include <test_module/test.hpp>

using namespace template_lib;

class MinimalUnsignedTypeTest : public TestGroupBase
{
};

TEST_F(MinimalUnsignedTypeTest, MinimalUnsignedType_WithValueFittingInU8_ResolvesToU8)
{
    using type1 = minimal_unsigned_type_t<0>;
    using type2 = minimal_unsigned_type_t<100>;
    using type3 = minimal_unsigned_type_t<std::numeric_limits<u8>::max()>;

    static_assert(std::is_same_v<type1, u8>);
    static_assert(std::is_same_v<type2, u8>);
    static_assert(std::is_same_v<type3, u8>);
    EXPECT_TRUE((std::is_same_v<type3, u8>));
}

TEST_F(MinimalUnsignedTypeTest, MinimalUnsignedType_WithValueFittingInU16_ResolvesToU16)
{
    using type1 = minimal_unsigned_type_t<std::numeric_limits<u8>::max() + 1>;
    using type2 = minimal_unsigned_type_t<10000>;
    using type3 = minimal_unsigned_type_t<std::numeric_limits<u16>::max()>;

    static_assert(std::is_same_v<type1, u16>);
    static_assert(std::is_same_v<type2, u16>);
    static_assert(std::is_same_v<type3, u16>);
    EXPECT_TRUE((std::is_same_v<type3, u16>));
}

TEST_F(MinimalUnsignedTypeTest, MinimalUnsignedType_WithValueFittingInU32_ResolvesToU32)
{
    using type1 = minimal_unsigned_type_t<std::numeric_limits<u16>::max() + 1>;
    using type2 = minimal_unsigned_type_t<1000000>;
    using type3 = minimal_unsigned_type_t<std::numeric_limits<u32>::max()>;

    static_assert(std::is_same_v<type1, u32>);
    static_assert(std::is_same_v<type2, u32>);
    static_assert(std::is_same_v<type3, u32>);
    EXPECT_TRUE((std::is_same_v<type3, u32>));
}

TEST_F(MinimalUnsignedTypeTest, MinimalUnsignedType_WithValueExceedingU32_ResolvesToU64)
{
    using type1 = minimal_unsigned_type_t<static_cast<size_t>(std::numeric_limits<u32>::max()) + 1>;
    using type2 = minimal_unsigned_type_t<0xFFFFFFFFFFFFFFFF>;

    static_assert(std::is_same_v<type1, u64>);
    static_assert(std::is_same_v<type2, u64>);
    EXPECT_TRUE((std::is_same_v<type2, u64>));
}
