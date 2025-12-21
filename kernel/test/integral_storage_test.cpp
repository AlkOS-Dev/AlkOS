#include <bit.hpp>
#include <bits_ext.hpp>
#include <limits.hpp>
#include <template/integral_storage.hpp>
#include <test_module/test.hpp>

using namespace template_lib;

class MinimalUnsignedStorageTest : public TestGroupBase
{
};

TEST_F(MinimalUnsignedStorageTest, MinimalUnsignedStorage_AllBoundariesAndEdgeCases)
{
    constexpr size_t val_1bit        = 1;
    constexpr size_t val_2bits       = 2;
    constexpr size_t val_4bits       = 15;
    constexpr size_t val_7bits       = 127;
    constexpr size_t val_8bits_max   = 255;
    constexpr size_t val_8bits_plus1 = 256;
    constexpr size_t val_9bits       = 511;
    constexpr size_t val_15bits      = 32767;
    constexpr size_t val_16bits_max  = 65535;
    constexpr size_t val_25bits      = 16777216;
    constexpr size_t val_31bits      = 2147483647;
    constexpr size_t val_32bits_max  = 4294967295;
    constexpr size_t val_63bits      = 9223372036854775807ULL;
    constexpr size_t val_64bits_max  = 18446744073709551615ULL;

    constexpr int bits_1bit        = std::bit_width(val_1bit);
    constexpr int bits_2bits       = std::bit_width(val_2bits);
    constexpr int bits_4bits       = std::bit_width(val_4bits);
    constexpr int bits_7bits       = std::bit_width(val_7bits);
    constexpr int bits_8bits_max   = std::bit_width(val_8bits_max);
    constexpr int bits_8bits_plus1 = std::bit_width(val_8bits_plus1);
    constexpr int bits_9bits       = std::bit_width(val_9bits);
    constexpr int bits_15bits      = std::bit_width(val_15bits);
    constexpr int bits_16bits_max  = std::bit_width(val_16bits_max);
    constexpr int bits_25bits      = std::bit_width(val_25bits);
    constexpr int bits_31bits      = std::bit_width(val_31bits);
    constexpr int bits_32bits_max  = std::bit_width(val_32bits_max);
    constexpr int bits_63bits      = std::bit_width(val_63bits);
    constexpr int bits_64bits_max  = std::bit_width(val_64bits_max);

    constexpr size_t bytes_1bit        = (bits_1bit + 7) / 8;
    constexpr size_t bytes_2bits       = (bits_2bits + 7) / 8;
    constexpr size_t bytes_4bits       = (bits_4bits + 7) / 8;
    constexpr size_t bytes_7bits       = (bits_7bits + 7) / 8;
    constexpr size_t bytes_8bits_max   = (bits_8bits_max + 7) / 8;
    constexpr size_t bytes_8bits_plus1 = (bits_8bits_plus1 + 7) / 8;
    constexpr size_t bytes_9bits       = (bits_9bits + 7) / 8;
    constexpr size_t bytes_15bits      = (bits_15bits + 7) / 8;
    constexpr size_t bytes_16bits_max  = (bits_16bits_max + 7) / 8;
    constexpr size_t bytes_25bits      = (bits_25bits + 7) / 8;
    constexpr size_t bytes_31bits      = (bits_31bits + 7) / 8;
    constexpr size_t bytes_32bits_max  = (bits_32bits_max + 7) / 8;
    constexpr size_t bytes_63bits      = (bits_63bits + 7) / 8;
    constexpr size_t bytes_64bits_max  = (bits_64bits_max + 7) / 8;

    EXPECT_EQ(1, bits_1bit);
    EXPECT_EQ(1, bytes_1bit);
    EXPECT_EQ(2, bits_2bits);
    EXPECT_EQ(1, bytes_2bits);
    EXPECT_EQ(4, bits_4bits);
    EXPECT_EQ(1, bytes_4bits);
    EXPECT_EQ(7, bits_7bits);
    EXPECT_EQ(1, bytes_7bits);
    EXPECT_EQ(8, bits_8bits_max);
    EXPECT_EQ(1, bytes_8bits_max);
    EXPECT_EQ(9, bits_8bits_plus1);
    EXPECT_EQ(2, bytes_8bits_plus1);
    EXPECT_EQ(9, bits_9bits);
    EXPECT_EQ(2, bytes_9bits);
    EXPECT_EQ(15, bits_15bits);
    EXPECT_EQ(2, bytes_15bits);
    EXPECT_EQ(16, bits_16bits_max);
    EXPECT_EQ(2, bytes_16bits_max);
    EXPECT_EQ(25, bits_25bits);
    EXPECT_EQ(4, bytes_25bits);
    EXPECT_EQ(31, bits_31bits);
    EXPECT_EQ(4, bytes_31bits);
    EXPECT_EQ(32, bits_32bits_max);
    EXPECT_EQ(4, bytes_32bits_max);
    EXPECT_EQ(63, bits_63bits);
    EXPECT_EQ(8, bytes_63bits);
    EXPECT_EQ(64, bits_64bits_max);
    EXPECT_EQ(8, bytes_64bits_max);

    using t_1bit        = MinimalUnsignedStorage_t<val_1bit>;
    using t_2bits       = MinimalUnsignedStorage_t<val_2bits>;
    using t_4bits       = MinimalUnsignedStorage_t<val_4bits>;
    using t_7bits       = MinimalUnsignedStorage_t<val_7bits>;
    using t_8bits_max   = MinimalUnsignedStorage_t<val_8bits_max>;
    using t_8bits_plus1 = MinimalUnsignedStorage_t<val_8bits_plus1>;
    using t_9bits       = MinimalUnsignedStorage_t<val_9bits>;
    using t_15bits      = MinimalUnsignedStorage_t<val_15bits>;
    using t_16bits_max  = MinimalUnsignedStorage_t<val_16bits_max>;
    using t_25bits      = MinimalUnsignedStorage_t<val_25bits>;
    using t_31bits      = MinimalUnsignedStorage_t<val_31bits>;
    using t_32bits_max  = MinimalUnsignedStorage_t<val_32bits_max>;
    using t_63bits      = MinimalUnsignedStorage_t<val_63bits>;
    using t_64bits_max  = MinimalUnsignedStorage_t<val_64bits_max>;

    static_assert(std::is_same_v<t_1bit, u8>);
    static_assert(std::is_same_v<t_2bits, u8>);
    static_assert(std::is_same_v<t_4bits, u8>);
    static_assert(std::is_same_v<t_7bits, u8>);
    static_assert(std::is_same_v<t_8bits_max, u8>);
    static_assert(std::is_same_v<t_8bits_plus1, u16>);
    static_assert(std::is_same_v<t_9bits, u16>);
    static_assert(std::is_same_v<t_15bits, u16>);
    static_assert(std::is_same_v<t_16bits_max, u16>);
    static_assert(std::is_same_v<t_25bits, u32>);
    static_assert(std::is_same_v<t_31bits, u32>);
    static_assert(std::is_same_v<t_32bits_max, u32>);
    static_assert(std::is_same_v<t_63bits, u64>);
    static_assert(std::is_same_v<t_64bits_max, u64>);

    EXPECT_TRUE((std::is_same_v<t_1bit, u8>));
    EXPECT_TRUE((std::is_same_v<t_2bits, u8>));
    EXPECT_TRUE((std::is_same_v<t_4bits, u8>));
    EXPECT_TRUE((std::is_same_v<t_7bits, u8>));
    EXPECT_TRUE((std::is_same_v<t_8bits_max, u8>));
    EXPECT_TRUE((std::is_same_v<t_8bits_plus1, u16>));
    EXPECT_TRUE((std::is_same_v<t_9bits, u16>));
    EXPECT_TRUE((std::is_same_v<t_15bits, u16>));
    EXPECT_TRUE((std::is_same_v<t_16bits_max, u16>));
    EXPECT_TRUE((std::is_same_v<t_25bits, u32>));
    EXPECT_TRUE((std::is_same_v<t_31bits, u32>));
    EXPECT_TRUE((std::is_same_v<t_32bits_max, u32>));
    EXPECT_TRUE((std::is_same_v<t_63bits, u64>));
    EXPECT_TRUE((std::is_same_v<t_64bits_max, u64>));
}
